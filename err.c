#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
void exit_err(int i,char *msg){
  if( i < 0 ) {
    printf("error #%d during [%s]: %s\n",errno,msg,strerror(errno));
    exit(errno);
  }
}
