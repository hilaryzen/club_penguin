#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

/* void exit_err(int i, char *msg) */
/* utility function across all methods: if i, a return value from a function, is negative, sends a signal to itself to terminate */

void exit_err(int i,char *msg){
  if( i < 0 ) {
    printf("error #%d during [%s]: %s\n",errno,msg,strerror(errno));
    kill( getpid(), SIGTERM );
    // not just exiting because i want the proper exit stuff to happen
  }
}
