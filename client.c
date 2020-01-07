#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>


#include "err.h"
#include "client.h"
#include "packets.h"

int touch_log = 1;

int main(int argc, char *argv[]){

  // DECLARATIONS
  int r; // temporary type variable to store return values that might indicate errors
  int sd; // descriptor for socket to the server
  char *host; // server IPv4 address
  fd_set readset; // buffer in which set of file descriptors for select() will be put
  /* buffers in which to put messages to be written onto the socket */
  struct packet_header header;
  union packet packet;
  struct cnx_header cnx_info; // struct which contains information about our connection
  struct cnx_header cnx_who_sent; //used with update_log
  // ARG INTERPRETATION
  if (argc == 1) host = LOCALHOST; // default server, when unspecified, is localhost (127.0.0.1)
  else if(*argv[1] == 'k') host = KHOSEKH; // if the second arg starts with 'k', it'll connect to kiran's droplet (just a convenience thing for me -k)
  else host = argv[1]; // if specified by user it goes here

  // CONFIGURE CONNECTION INFO
  // default indications, such as your username, etc, would go here
  printf("username: ");
  fgets(cnx_info.username,16,stdin);
  *(strchr(cnx_info.username,'\n')) = '\0'; // eliminate newline
  cnx_info.room = 0;

  //BEFORE YOU CONNECT TO SERVER, SET UP YOUR CHAT LOG
  if (create_log()){
    //if this fails, change touch_log to 0 so we don't update a log that isn't existing
    touch_log = 0;
    printf("uh oh, create_log malfunctioned -- touch_log = 0\n");
  }
  // CONNECT TO THE SERVER
  sd = client_setup(host);
  printf("[client] setup on sd %d\n",sd);
  write(sd,&cnx_info,sizeof(struct cnx_header));
  printf("[client] wrote connection header\n");

  // forever loop: as data comes in, interpret keyboard input or server messages
  while (1) {
    // 1. PRINT CURRENT STATE
    //   -right now this is just printing a new prompt
    //   -unless we use sdl or something, we could use this space to reprint the current state of the game/messages

    fflush(stdin); // i do not understand what this does but it was in mr dw's code and stuff acts weird without it
    printf("enter data: ");

    // 2. SELECT(): reset set configuration and then wait for one of the internal sd's to have data; either stdin or sd
    reset_fdset(&readset,sd);
    select(sd+1,&readset,NULL,NULL,NULL);

    // IF STDIN IS READY: HANDLE USER INPUT
    if (FD_ISSET(STDIN_FILENO,&readset)) {
      // get the message, remove the newline
      fgets(packet.CHATMSG.message,80,stdin);
      *strchr(packet.CHATMSG.message,'\n') = '\0';
      //remember to re-add that \n when you add this line to your log.txt ^^

      // configure header to contain proper information about the packet
      header.packet_type = P_CHATMSG;
      header.packet_size = sizeof(struct chatmsg);
      // write the header and packet to the server socket
      write(sd,&header,sizeof(struct packet_header));
      write(sd,&packet,sizeof(struct chatmsg));
    }

    // IF SOCKET IS READY: HANDLE SERVER MESSAGE
    //alma edit -- i want the server to send cnx_info packet too so we can make log look like:
    //username: hey!
    //username2: sup?
    if (FD_ISSET(sd,&readset)) {
      printf("[client] received packet from server\n");
      r = read(sd,&header,sizeof(struct packet_header));
      // this is my current makeshift way of checking whether the received message is an end of file
      if(r != sizeof(struct packet_header)){
	       printf("eof i think\n");
	        exit(0);
      }
      //get info of who sent
      read(sd, &cnx_who_sent, sizeof(struct cnx_header));
      char *who_sent = cnx_who_sent.username;
      //
      read(sd,&packet,header.packet_size);
      // in the place of this print, there would be handling of every type of packet here, updating the game state as necessary
      printf("message: [%s]\n",packet.CHATMSG.message);
      //in the future, this will be contained in an if statement (if packet_header.packet_type == CHATMSG). for now we r only sending chats
      if (touch_log){
        r = update_log(packet.CHATMSG.message, who_sent);
        if (r != 0){
          printf("update_log not working\n");
        }
      }
      //print and reprint below

    }
  }
  return 0;
}

/* thank you mr dw! this is entirely based on dwsource/networking */
int client_setup(char *server){
  int sd,r;

  // create an internet socket (AF_INET, aka internet, means it expects an IPv4 address), not yet connected to anything
  sd = socket( AF_INET, SOCK_STREAM, 0 );
  exit_err(sd,"create client socket");

  // configure a struct with the settings we want for the server (hints)
  struct addrinfo *hints,*results;
  hints = (struct addrinfo *)calloc(1,sizeof(struct addrinfo)); // calloc'd so its guaranteed to be zero'd out at the beginning
  hints->ai_family = AF_INET; // IPv4 type address
  hints->ai_socktype = SOCK_STREAM; //TCP socket (some protocol, i think we're gonna learn about it soon in class as of 2020-01-05)
  /* uses the user's `server` string + our settings in `hints` to create a properly formatted address, stored in `results` */
  getaddrinfo(server,PORT,hints,&results);

  // using the address/length generated above, connect our socket to the server
  r = connect( sd, results->ai_addr, results->ai_addrlen );
  exit_err(r,"client connect");

  // free the resources we used
  free(hints);
  freeaddrinfo(results);

  return sd;
}

void reset_fdset(fd_set *set,int sd){
  /* uses the macro's specified in [man select] to set up a new fd set containing both stdin and the specified additional descriptor in `sd` */
  FD_ZERO(set);
  FD_SET(STDIN_FILENO,set);
  FD_SET(sd,set);
}

int update_log(char *addition, char *who_sent){
  int fd = open("log.txt", O_WRONLY | O_APPEND);
  exit_err(fd, "tried opening update_log");
  strcat(who_sent, "\e[%d;%df%s:\t");
  int len_write = strlen(who_sent);
  write(fd, who_sent, len_write);
  strcat(addition, "\n");
  len_write = strlen(addition); //so this should also add the \n
  write(fd, addition, len_write);
  exit_err(close(fd), "couldn't close log.txt in update_log");
  return 0;
}

int create_log(){
  //only the user shld be able to interact w log
  //also, i don't want to recreate the log if for somereason client runs main over again
  int fd = open("log.txt", O_CREAT | O_EXCL, 0600);
  if (fd == -1){
    return 1; //so if this file already exists (2+ clients in same direcotry), don't make it again
  }
  return 0;
}
