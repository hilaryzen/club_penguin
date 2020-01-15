#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>

#include "err.h"
#include "client.h"
#include "packets.h"
#include "windows.h"

int touch_log = 1;
int sd; // descriptor for socket to the server
struct cnx_header cnx_info; // struct which contains information about our connection

int main(int argc, char *argv[]){

  // DECLARATIONS
  int r; // temporary type variable to store return values that might indicate errors
  char *host; // server IPv4 address
  fd_set readset; // buffer in which set of file descriptors for select() will be put
  /* buffers in which to put messages to be written onto the socket */
  struct packet_header header;
  union packet packet;
  /* ncurses windows for typing buffer, game display, and chat window */
  WINDOW *game_win;
  WINDOW *chat_win;
  WINDOW *type_win;
  int log_fd; // file descriptor for log.txt
  
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
  char log_filename[16];
  sprintf(log_filename,"%d_log.txt",getpid());
  log_fd = open(log_filename,O_CREAT|O_EXCL|O_TRUNC|O_RDWR,0600);
  exit_err(log_fd,"open log file");
  printf("value of log_fd: %d\n",log_fd);
  r = write(log_fd,"CHAT START:\n",12);
  exit_err(r,"write to log");
  // CONNECT TO THE SERVER
  sd = client_setup(host);
  printf("[client] setup on sd %d\n",sd);
  write(sd,&cnx_info,sizeof(struct cnx_header));
  printf("[client] wrote connection header\n");

  // CONFIGURE WINDOWS
  exit_err( setup(&game_win,&chat_win,&type_win) , "configure ncurses windows" );
  
  // (from read_from_type(): configure text input spacing
  keypad(type_win, TRUE);
  scrollok(type_win, TRUE); //so if we've printed out of the window, will just scroll down
  wmove(type_win, 0, 0); //set cursor
  char message[128] = "";
  //char *to_send;
  int i = 0;
  int size = 0;

  // forever loop: as data comes in, interpret keyboard input or server messages
  while (1) {
    // 1. PRINT CURRENT STATE
    //   -right now this is just printing a new prompt
    //   -unless we use sdl or something, we could use this space to reprint the current state of the game/messages

    // 2. SELECT(): reset set configuration and then wait for one of the internal sd's to have data; either stdin or sd
    reset_fdset(&readset,sd);
    select(sd+1,&readset,NULL,NULL,NULL);

    // IF STDIN IS READY: HANDLE USER INPUT
    if (FD_ISSET(STDIN_FILENO,&readset)) {
      if( read_from_type(&type_win,&chat_win,&chat_win,message,&i,&size) ) break;
    }

    // IF SOCKET IS READY: HANDLE SERVER MESSAGE
    if (FD_ISSET(sd,&readset)) {
      // printf("[client] received packet from server\n");
      r = read(sd,&header,sizeof(struct packet_header));
      // this is my current makeshift way of checking whether the received message is an end of file
      if(r != sizeof(struct packet_header)){
	       printf("eof i think\n");
	        exit(0);
      }
      read(sd,&packet,header.packet_size);
      // in the place of this print, there would be handling of every type of packet here, updating the game state as necessary
      // printf("message: [%s]\n",packet.CHATMSG.message);
      //in the future, this will be contained in an if statement (if packet_header.packet_type == CHATMSG). for now we r only sending chats

      char msgbuffer[256];
      sprintf(msgbuffer,"%s:%s",header.username,packet.CHATMSG.message);
      add_to_log(msgbuffer,strlen(msgbuffer),log_fd);
      print_log(&chat_win,log_fd);
      wrefresh(type_win);
    }
  }

  if (cleanup(&game_win, &chat_win, &type_win)){
    printf("uh oh, cleanup failed\n");
  }
  //sleep(2); just did this to test that cleanup works
  endwin();			/* End curses mode		  */

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

void sendchat(char *msg){
  struct packet_header header;
  struct chatmsg message;
  //alma adding that you fill in header to have username too
  strncpy(header.username, cnx_info.username,16);
  strncpy(message.message,msg,128);
  //
  header.packet_type = P_CHATMSG;
  header.packet_size = strlen(message.message);
  // write the header and packet to the server socket
  write(sd,&header,sizeof(struct packet_header));
  write(sd,&message,header.packet_size);
}
