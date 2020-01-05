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

int SEMD;
int SHMD;
struct cnx_header *SHM;

int main(){
  // declarations 
  int client_sockets[MAX_CNX];
  int queue_handler = 0;
  int f,r,i;
  // create socket listening over internet
  int listen_socket = server_setup();
  // create queue pipe
  int queue[2];
  r = pipe(queue);
  exit_err(r,"create queue pipe");
  // configure/attach shared memory
  SHMD = shmget(KEY, MAX_CNX * sizeof(struct cnx_header), IPC_CREAT|IPC_EXCL|0644);
  exit_err(SHMD,"creating shared memory");
  SHM = shmat(SHMD,0,0);
  for(i=0;i<MAX_CNX;i++) SHM[i].id = -1;

  SEMD = sem_config(KEY,IPC_CREAT|IPC_EXCL|0644,NSEMS,MAX_CNX,1,1);

  // signal handling
  signal(SIGTERM,main_sighandler);
  signal(SIGINT,main_sighandler);
  
  while (1) {
    int client_sd = server_connect(listen_socket);
    struct cnx_header cnx_info;
    r = read(client_sd,&cnx_info,sizeof(struct cnx_header));
    exit_err(r,"read cnx header");
    sem_claim(SEMD,SHM_SEMA);
    int id = clean_id();
    SHM[id] = cnx_info;
    SHM[id].id = id;
    SHM[id].sd = client_sd;
    sem_release(SEMD,SHM_SEMA);
    f = fork();
    if(!f){
      signal(SIGTERM,subserver_sighandler);
      signal(SIGINT,subserver_sighandler);
      child_init_ipc();
      close(queue[READ]);
      printf("[subserver %d] about to listen\n",getpid());
      subserver_listen(id,queue[WRITE]);
      exit(0);
    }
    // refresh queue handler
    if(queue_handler) kill(queue_handler,SIGTERM);
    queue_handler = fork();
    if(!queue_handler){
      signal(SIGTERM,queue_sighandler);
      signal(SIGINT,queue_sighandler);
      signal(SIGPIPE,queue_sighandler);
      child_init_ipc();
      close(queue[WRITE]);
      process_queue(queue[READ]);
      exit(0);
    }
  }
}

void process_queue(int qd){
  struct packet_header header;
  union packet packet;
  int i;
  printf("(just out of curiosity but also this means the queue restarted: sizeof( enum packet_t ) == %ld\n",sizeof(enum packet_t));
  while( read(qd,&header,sizeof( struct packet_header )) ){
    printf("[queue handler %d] handling packet\n",getpid());
    read(qd,&packet,header.packet_size);
    for( i=0; i<MAX_CNX; i++ ){
      printf("[%d]: id=%d, sd=%d",i,SHM[i].id,SHM[i].sd);
      if( SHM[i].id>=0 && should_receive(SHM+i,&header,&packet) ){
	printf(": writing\n");
	write( SHM[i].sd, &header, sizeof( struct packet_header ) );
	write( SHM[i].sd, &packet, header.packet_size );
      }
      printf("\n");
    }
  }
}
void subserver_listen(int id,int qd){
  int r;
  struct packet_header header;
  union packet packet;
  printf("[subserver %d] ready to listen: id %d(%d), sd %d\n",getpid(),SHM[id].id,id,SHM[id].sd);
  while( r = read(SHM[id].sd,&header,sizeof( struct packet_header )) ){
    exit_err(r,"socket read");
    if( r != sizeof(struct packet_header) ){
      exit(0);
    }
    printf("[subserver %d] received a packet\n",getpid());
    read(SHM[id].sd,&packet,header.packet_size);
    process(SHM+id,&header,&packet,qd);
  }
  printf("[subserver %d] eof reached\n",getpid());
  sem_claim(SEMD,SHM_SEMA);
  SHM[id].id = -1;
  close(SHM[id].sd);
  sem_release(SEMD,SHM_SEMA);
  shmdt(SHM);
  exit(0);
}

void qwrite(struct packet_header *header,union packet *packet, int qd){
  sem_claim(SEMD,QUEUE_SEMA);
  write( qd, header, sizeof( struct packet_header ) );
  write( qd, packet, header->packet_size );
  sem_release(SEMD,QUEUE_SEMA);
}

int clean_id(){
  /* should not be used without having first gotten the sem! ! ! */
  int i = 0;
  while( i < MAX_CNX && SHM[i].id>=0 ) i++;
  return i==MAX_CNX ? -1 : i;
}

void child_init_ipc(){
  SHMD = shmget(KEY,0,0);
  exit_err(SHMD,"connecting to shared memory");
  SHM = shmat(SHMD,0,0);
  SEMD = sem_connect(KEY,NSEMS);  
}

/* this is almost exactly the dwsource code when we learn more maybe ill change it lol
thanks mr dw! */

int server_setup(){
  int sd,r;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  exit_err( sd, "creating server socket" );
  printf("[server] socket created\n");

  struct addrinfo * hints, * results;
  hints = (struct addrinfo *)calloc(1,sizeof(struct addrinfo));
  hints->ai_family = AF_INET; // IPv4 address type
  hints->ai_socktype = SOCK_STREAM; //TCP socket
  hints->ai_flags = AI_PASSIVE; // listen on any valid address
  getaddrinfo(NULL,PORT,hints,&results);

  r = bind( sd, results->ai_addr, results->ai_addrlen );
  exit_err( r, "server bind" );
  printf("[server] socket bound\n");
  
  // set socket to listen state
  r = listen(sd,10);
  exit_err( r , "server listen" );
  printf("[server] socket in listen state\n");

  freeaddrinfo(results);
  free(hints);
  return sd;
}

int server_connect(int sd){
  int client_socket;
  socklen_t sock_size;
  struct sockaddr_storage client_address;
  sock_size = sizeof(client_address);
  client_socket = accept(sd, (struct sockaddr *)&client_address, &sock_size);
  exit_err(client_socket, "server accept");
  printf("[server] client accepted\n");
  return client_socket;
}

void main_sighandler(int signal){
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
  switch(signal){
  case SIGINT: case SIGTERM:
    printf("[qhandler %d]shutting down\n",getpid());
    shmdt(SHM);
    exit(SIGTERM);
    break;
  case SIGPIPE:
    printf("[qhandler %d]broken pipe\n",getpid());
    break;
  default:
    break;
  }
}
