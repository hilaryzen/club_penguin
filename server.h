#ifndef SERVER_H
#define SERVER_H

#define MAX_CNX 8
#define KEY 91289

#define CNX_SEMA 0
#define SHM_SEMA 1
#define QUEUE_SEMA 2

/* FOR EXTERNAL USE */

/* utility */
int clean_id();

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
