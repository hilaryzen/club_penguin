#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

int server_setup(){
  int sd,r;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  exit_err( sd, "creating server socket" );
  printf(" primary socket created\n");

  struct addrinfo * hints, * results;
  hints = (struct addrinfo *)calloc(1,sizeof(struct addrinfo));
  hints->ai_family = AF_INET; // IPv4 address type
  hints->ai_socktype = SOCK_STREAM; //TCP socket
  hints->ai_flags = AI_PASSIVE; // listen on any valid address
  getaddrinfo(NULL,PORT,&hints,&results);

  i = listen(sd,10);
  
  freeaddrinfo(results);
  free(hints);
}

int server_connect(int sd);
