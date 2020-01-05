#ifndef CLIENT_H
#define CLIENT_H

#define LOCALHOST "127.0.0.1"
#define KHOSEKH "209.97.155.54"

int main(int argc, char *argv[]);

void reset_fdset(fd_set *set, int sd); // zeroes `set` and puts stdin and the specified socket descriptor in

int client_setup(char *server);

/*
Will add any message sent out or recieved to your chat log file
Later, develop method to display the log on screen
Later later, develop scrolling

0 on success
*/
int update_log(char *addition);

/*
makes log.txt, which is updated every time you recieve a message
additions should be in this format:
(username): message \n

0 on success
*/
int create_log();


#endif
