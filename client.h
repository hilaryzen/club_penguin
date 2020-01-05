#ifndef CLIENT_H
#define CLIENT_H

#define LOCALHOST "127.0.0.1"
#define KHOSEKH "209.97.155.54"

int main(int argc, char *argv[]);

void reset_fdset(fd_set *set, int sd); // zeroes `set` and puts stdin and the specified socket descriptor in

int client_setup(char *server);

#endif
