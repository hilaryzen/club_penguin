#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "err.h"
#include "packets.h"
#include "sema.h"
#include "server.h"

int SEMD;
int SHMD;
struct cnx_header *SHM;

int main(){
  // declarations 
  int client_sockets[MAX_CNX];
  int merger_pid = 0;
  int f,r;
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
  exit_err(SHM,"attaching to memory");

  SEMD = sem_config(KEY,IPC_CREAT|IPC_EXCL|0644,3,MAX_CNX,1,1);
  
  while (1) {
    
  }
}


int clean_id(){
  /* should not be used without having first gotten the sem! ! ! */
  int i = 0;
  while( i < MAX_CNX && SHM[i].id ) i++;
  return i==MAX_CNX ? -1 : i;
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
}

int server_connect(int sd){
  int client_socket;
  socklen_t sock_size;
  struct sockaddr_storage client_address;
  sock_size = sizeof(client_address);
  client_socket = accept(sd, (struct sockaddr *)&client_address, &sock_size);
  exit_err(client_socket, "server accept");
  printf("[server] client accepted\n");
}
