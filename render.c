#include <stdio.h>
#include <stdlib.h>
#include "render.h"

/* none of this is in use yet! if we don't use a library, this is a good place to put utility stuff */

void resize_term(){
  printf("%s\e[8;%d;%dt",CLEAR_TERMINAL,ROWCOUNT,COLCOUNT);
}

void draw_penguin(int r,int c){
  printf("\e[%d;%df%s",r,c,PENGUIN);
}
