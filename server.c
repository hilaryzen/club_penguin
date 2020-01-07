#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "err.h"
#include "packets.h"
#include "sema.h"
#include "server.h"
#include "processor.h"

// STATIC VARIABLES, accessible throughout the file!
// by putting this stuff here, its still accessible by the signal handlers and stuff can get removed on an abnormal exit
int SEMD;
int SHMD;
struct cnx_header *SHM;

int main(){
  // DECLARATIONS
  int queue_handler = 0; // will store the pid of the process currently handling outbox sends, so that it can be killed and restarted
  int listen_socket; // descriptor for the main socket, where new connections will come in
  int queue[2]; // array in which the two ends of the pipe for merging outbound messages from all subservers
  int f,r,i; // f as a temp for fork(); r as a temp for return values; i as an index counter
  struct cnx_header cnx_info; // buffer for connection information for new clients

  // CONFIGURATION
  // create socket listening for new connections
  listen_socket = server_setup();
  // create outbox queue pipe
  r = pipe(queue);
  exit_err(r,"create queue pipe");
  // configure/attach shared memory
  SHMD = shmget(KEY, MAX_CNX * sizeof(struct cnx_header), IPC_CREAT|IPC_EXCL|0644);
  exit_err(SHMD,"creating shared memory");
  SHM = shmat(SHMD,0,0);
  for(i=0;i<MAX_CNX;i++) SHM[i].id = -1;
  // configure semaphore array with NSEMS semaphores, the first with MAX_CNX as an initial value and the rest with one
  // semaphore 0, aka CNX_SEMA, makes sure not too many connections are received
  // semaphore 1, aka SHM_SEMA, must be taken before modifying the contents of the shared memory
  // semaphore 2, aka QUEUE_SEMA, must be taken before writing to the outbox queue
  SEMD = sem_config(KEY,IPC_CREAT|IPC_EXCL|0644,NSEMS,MAX_CNX,1,1);

  // signal handling: link the necessary signals to processes as necessary
  signal(SIGTERM,main_sighandler);
  signal(SIGINT,main_sighandler);

  // FOREVER LOOP: wait for a new user to try and connect, then fork a subserver to handle their input and refresh the queue handler with the new socket
  while (1) {
    // block until there are few enough connections for a new connection
    sem_claim(SEMD,CNX_SEMA);
    // block until a connection is initiated
    int client_sd = server_connect(listen_socket);
    // from the new socket, read the first data from the client, their connection information
    r = read(client_sd,&cnx_info,sizeof(struct cnx_header));
    exit_err(r,"read cnx header");
    // get an ID for the user and store their connection information in shared memory
    sem_claim(SEMD,SHM_SEMA);
    int id = clean_id();
    SHM[id] = cnx_info;
    SHM[id].id = id; // this indicates that the spot is now in use, id is no longer -1
    SHM[id].sd = client_sd;
    sem_release(SEMD,SHM_SEMA);
    // fork off the subserver process!
    f = fork();
    if(!f){
      // configure signal handling in a new way for this subserver
      signal(SIGTERM,subserver_sighandler);
      signal(SIGINT,subserver_sighandler);
      // attach to shared memory and get a shm id
      child_init_ipc();
      // close the read end of this pipe because we will be writing to it
      close(queue[READ]);
      printf("[subserver %d] about to listen\n",getpid());
      //gonna print the username
      printf("username of user connected: %s\n", SHM[id].username);
      subserver_listen(id,queue[WRITE]);
      // this shouldn't be necessary but if subserver_listen returns we shouldn't let it continue on to become an extra main server that'd be bad
      exit(0);
    }
    // kill the old queue handler if there is one
    if(queue_handler) kill(queue_handler,SIGTERM);
    // fork a new queue handler
    /* (this is necessary because the queue handler needs to have the socket we just created in this process, and the old one lacks the socket */
    queue_handler = fork();
    if(!queue_handler){
      // configure signal handling in a new way for this process
      signal(SIGTERM,queue_sighandler);
      signal(SIGINT,queue_sighandler);
      signal(SIGPIPE,queue_sighandler); // this isn't used yet but i could see it being useful
      // attach to shared memory and get a shm id
      child_init_ipc();
      // close the write end of this pipe since we will be reading from it in this process
      close(queue[WRITE]);
      process_queue(queue[READ], id);
      // shouldn't be reached but a safety net so no unexpected behavior is possible here
      exit(0);
    }
  }
  return 0;
}
//** i (alma) modified this so the cnx_info of who sent this goes w the message
void process_queue(int qd){
  // OUTBOX QUEUE HANDLER PROCESS
  /* this process dispatches messages written to the outbox queue to users */
  /* who the messages are sent to is determined by processor.c:should_receive() */

  // DECLARATIONS: space in which to read in packets from the queue
  struct packet_header header;
  union packet packet;
  int i; // index incrementer
  printf("[qhandler %d] start\n",getpid());
  // unless the queue is destroyed somehow, reads header and packet from the queue
  while( read(qd,&header,sizeof( struct packet_header )) ){
    printf("[qhandler %d] handling packet\n",getpid());
    read(qd,&packet,header.packet_size);
    for( i=0; i<MAX_CNX; i++ ){
      /* foreach connection space:
	     if there is a connection in this space and it should receive this packet,
	     then write the packet to its socket */
       //Alma -- im adding sending the username of whoever sent this message so it can
       //be added to the log
      if( SHM[i].id>=0 && should_receive(SHM+i,&header,&packet) ){
      	write( SHM[i].sd, &header, sizeof( struct packet_header ) );
        //is this bad? sending thru the socket our struct (cnx_header) --> before sending this client's socket, now sending socket of who sent
        //i get problem. this is sending message server got to client w that client's username
        //instead need to get the username into the message in subserver_listen
        write(SHM[i].sd, &SHM[header.id], sizeof( struct cnx_header)); //not sure how to get j the username from this
        //
        write( SHM[i].sd, &packet, header.packet_size );
      }
    }
  }
  // i don't think this would ever be reached but just in case
  exit(0);
}
void subserver_listen(int id,int qd){
  /* one instance of this is initiated for each connected user */
  /* listens on connection `id`'s socket for incoming messages*/
  /* calls `process()` to interpret the message and write the socket to the queue */
  // DECLARATIONS
  int r; // temp value for error indicating return values
  /* buffers in which to write messages from client and messages to be written out */
  struct packet_header header;
  union packet packet;
  printf("[subserver %d] ready to listen: id %d(%d), sd %d\n",getpid(),SHM[id].id,id,SHM[id].sd);
  // loops until socket closes: read message and process it (write messages to outbox)
  while( r = read(SHM[id].sd,&header,sizeof( struct packet_header )) ){
    if( r != sizeof(struct packet_header) ) break; // if not enough data is sent or there's an error, leave the loop
    printf("[subserver %d] received a packet\n",getpid());
    // read contents of packet from the socket
    read(SHM[id].sd,&packet,header.packet_size);
    // call `process`, as defined in processor.c:process()
    // `qd` can be used inside in order to call qwrite()
    process(SHM+id,&header,&packet,qd);
  }
  // end of socket, or an error in reading the header
  printf("[subserver %d] socket is gone or bad data sent\n",getpid());
  // update the shared memory to indicate this id space as empty
  sem_claim(SEMD,SHM_SEMA);
  SHM[id].id = -1;
  sem_release(SEMD,SHM_SEMA);
  close(SHM[id].sd);
  sem_release(SEMD,CNX_SEMA); // release the semaphore spot for total connections
  shmdt(SHM);
  printf("[subserver %d] shutting down\n",getpid());
  exit(0);
}

void qwrite(struct packet_header *header,union packet *packet, int qd){
  /* claim the outbox queue semaphore and write a header/packet pair to it */
  sem_claim(SEMD,QUEUE_SEMA);
  write( qd, header, sizeof( struct packet_header ) );
  write( qd, packet, header->packet_size );
  sem_release(SEMD,QUEUE_SEMA);
}

int clean_id(){
  /* return an id space that is not already occupied */
  /* if none is found, returns -1 instead */
  /* should not be used without having first gotten the sem! ! ! */
  int i = 0;
  while( i < MAX_CNX && SHM[i].id>=0 ) i++;
  return i==MAX_CNX ? -1 : i;
}

void child_init_ipc(){
  /* attach to shared memory and semaphore, store id's and pointers in the globals */
  SHMD = shmget(KEY,0,0);
  exit_err(SHMD,"connecting to shared memory");
  SHM = shmat(SHMD,0,0);
  SEMD = sem_connect(KEY,NSEMS);
}

/* this is almost exactly the dwsource code when we learn more maybe ill change it lol
thanks mr dw! */

int server_setup(){
  int sd,r;

  // configure an internet socket (AF_INET => Internet, IPv4 specifically)
  // not yet attached to anything
  sd = socket( AF_INET, SOCK_STREAM, 0 );
  //note: you only exit if sd < 0
  exit_err( sd, "creating server socket" );
  printf("[server] socket created\n");

  // configure object with the `settings` we want for our connection
  struct addrinfo * hints, * results;
  hints = (struct addrinfo *)calloc(1,sizeof(struct addrinfo)); // calloc'd to guarantee zero'd out memory
  hints->ai_family = AF_INET; // IPv4 address type
  hints->ai_socktype = SOCK_STREAM; //TCP socket (what is this??)
  hints->ai_flags = AI_PASSIVE; // listen on any valid address (localhost or your IP i think)
  /*stores an actual address, based on the settings set in hints, into results */
  getaddrinfo(NULL,PORT,hints,&results);

  // bind the socket we made to the address just generated
  r = bind( sd, results->ai_addr, results->ai_addrlen );
  exit_err( r, "server bind" );
  printf("[server] socket bound\n");

  // set socket to listen state
  r = listen(sd,10); // 10 is the number of connections that can be backlogged waiting to connect
  exit_err( r , "server listen" );
  printf("[server] socket in listen state\n");

  // free the resources used when we called getaddrinfo()
  freeaddrinfo(results);
  free(hints);
  return sd;
  // im really proud of writing my own comments on this stuff :) -kiran
}

int server_connect(int sd){
  /* when a user is connecting, crerate a socket communicating with just this user */
  int client_socket;
  socklen_t sock_size;
  struct sockaddr_storage client_address;
  sock_size = sizeof(client_address);

  /* blocks until a client is present to connect */
  /* once a client is there, stores their IPv4 adress in `client_address` */
  client_socket = accept(sd, (struct sockaddr *)&client_address, &sock_size);
  exit_err(client_socket, "server accept");

  printf("[server] client accepted\n");
  //is returning the descriptor of the socket
  return client_socket;
}

void main_sighandler(int signal){
  // on exit by any means, the main server should destroy the shared memory
  switch(signal){

  case SIGINT: case SIGTERM:
    printf("[server %d]shutting down\n",getpid());
    sem_remove(SEMD);
    shmdt(SHM);
    shmctl(SHMD,IPC_RMID,0);
    exit(SIGTERM);
    break;

  default:
    break;
  }
}
void subserver_sighandler(int signal){
  // on exit by any means, the subserver should detach from memory
  switch(signal){

  case SIGINT: case SIGTERM:
    printf("[subserver %d]shutting down\n",getpid());
    shmdt(SHM);
    exit(SIGTERM);
    break;

  default:
    break;
  }
}
void queue_sighandler(int signal){
  // on exit by any means, the queue handler should detach from shared memory
  switch(signal){
  case SIGINT: case SIGTERM:
    printf("[qhandler %d]shutting down\n",getpid());
    shmdt(SHM);
    exit(SIGTERM);
    break;
  case SIGPIPE:
    // SIGPIPE i think should be ignored, since there are several sockets she has to write to
    printf("[qhandler %d]broken pipe\n",getpid());
    break;
  default:
    break;
  }
}
