#ifndef SERVER_H
#define SERVER_H

/* FOR EXTERNAL USE */


/* this is ... weird lmao */
int main();

/* MY SERVER STRUCTURE */
void subserver_listen();

void process_queue();

/* ACTUAL NETWORKING STUFF */

/* 
   creates, binds a server side socket and sets it to listening state
   returns the socket descriptor
*/
int server_setup();

/*
    sd is the socket created in server_setup()
    waits for a connection, then sets up a socket connected to the client
    returns socket descriptor for the new socket
*/
int server_connect(int sd);

#endif
