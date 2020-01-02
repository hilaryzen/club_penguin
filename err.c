#include <stdio.h>
#include <stdlib.h>

void exit_err(int i,char *msg){
  if( i < 0 ) {
    printf("error #%s during [%s]: %s",errno,msg,strerror(errno));
    exit(errno);
  }
}
