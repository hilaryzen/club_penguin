#include <stdio.h>
#include <stdlib.h>
#include "render.h"

void resize_term(){
  printf("%s\e[8;%d;%dt",CLEAR_TERMINAL,ROWCOUNT,COLCOUNT);
}

void draw_penguin(int r,int c){
  printf("\e[%d;%df%s",r,c,PENGUIN);
}
