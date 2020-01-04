#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "err.h"
#include "client.h"
#include "packets.h"

int main(int argc, char *argv[]){
  int r;
  char *host;
  char buf[80];
  struct packet_header header;
  union packet packet;
  if (argc == 1) host = LOCALHOST;
  else if(*argv[1] == 'k') host = KHOSEKH;
  else host = argv[1];
  struct cnx_header cnx_info;
  strcpy(cnx_info.username,"kiran");
  cnx_info.room = 0;
  int sd = client_setup(host);
  printf("[client] setup on sd %d\n",sd);
  printf("[client] about to write a header\n");
  write(sd,&cnx_info,sizeof(struct cnx_header));
  printf("[client] wrote a header\n");
  while (1) {
    printf("enter data: ");
    fflush(stdout);
    fd_set select_read;
    FD_ZERO(&select_read);
    FD_SET(STDIN_FILENO,&select_read);
    FD_SET(sd,&select_read);
    select(sd+1,&select_read,NULL,NULL,NULL);
    if (FD_ISSET(STDIN_FILENO,&select_read)) {
      struct chatmsg chatmsg;
      fgets(chatmsg.message,80,stdin);
      *strchr(chatmsg.message,'\n') = '\0';
      header.packet_type = P_CHATMSG;
      header.packet_size = sizeof(struct chatmsg);
      packet.CHATMSG = chatmsg;
      write(sd,&header,sizeof(struct packet_header));
      write(sd,&packet,sizeof(struct chatmsg));
    }
    if (FD_ISSET(sd,&select_read)) {
      printf("[client] received packet from server\n");
      r = read(sd,&header,sizeof(struct packet_header));
      if(r != sizeof(struct packet_header)){
	printf("eof i think\n");
	exit(0);
      }
      read(sd,&packet,header.packet_size);
      printf("message: [%s]\n",packet.CHATMSG.message);
    }
  }
  return 0;
}

/* thank you mr dw! this is largely based on dwsource/networking */
int client_setup(char *server){
  int sd,r;

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  exit_err(sd,"create client socket");

  struct addrinfo *hints,*results;
  hints = (struct addrinfo *)calloc(1,sizeof(struct addrinfo));
  hints->ai_family = AF_INET; // IPv4
  hints->ai_socktype = SOCK_STREAM; //TCP socket
  getaddrinfo(server,PORT,hints,&results);

  r = connect( sd, results->ai_addr, results->ai_addrlen );
  exit_err(r,"client connect");

  free(hints);
  freeaddrinfo(results);

  return sd;
}
