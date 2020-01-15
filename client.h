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
int update_log(char *addition, char *who_sent);

/*
makes log.txt, which is updated every time you recieve a message
additions should be in this format:
(username): message \n
0 on success
IF LOG ALREADY EXISTS SHOULD SET A FLAG 'TOUCHING LOG' TO FALSE! AKA 0 -- THAT WAY ONLY 1 CLIENT HAS TO AFFECT LOG
*/
int create_log();

void sendchat(char *msg, int size);
#endif
