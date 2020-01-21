#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "windows.h"
#include "client.h"
#include "err.h"
#include "packets.h"
//PATH = "log.txt"

void insertchar(char *buf,int i,char c){
  int j = strlen(buf);
  buf[j+1] = '\0';
  while(j > i){
    buf[j] = buf[--j];
  }
  buf[i] = c;
}

void deletechar(char *buf,int i){
  while(buf[i]){
    buf[i] = buf[++i];
  }
}

WINDOW *create_newwin(int height, int width, int starty, int startx){
  WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	//box(local_win, 0 , 0);		/* 0, 0 gives default characters
	//				 * for the vertical and horizontal
	//				 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

int arrow_game(WINDOW *game_win, struct playermove *me){
  int ch = wgetch(game_win);
  switch(ch){
  case KEY_UP:
    if (me->r > 1) {
      me->r -= 1;
      return 1;
    }
    return 0;
  case KEY_DOWN:
    if (me->r < LINES - 3) {
      me->r += 1;
      return 1;
    }
    return 0;
  case KEY_LEFT:
    if (me->c > 0) {
      me->c -= 1;
      return 1;
    }
    return 0;
  case KEY_RIGHT:
    if (me->c < COLS / 2 - 3) {
      me->c += 1;
      return 1;
    }
    return 0;
  case KEY_F(1):
    return -1;
  case KEY_F(3):
    return 0;
  }
  return 0;
}

int read_from_type(WINDOW **type_win, WINDOW **chat_win, WINDOW **game_win,char *message, int *ind,int *sz){
  int i = *ind;
  int size = *sz;
  //remember we are catching special keys, called keypad(win, TRUE) in setup
  int ch = wgetch(*type_win); //get what the user puts down
  if (i < 126 && ch != '\n'){
    if (!has_key(ch)){
      insertchar(message,i,ch);
      i++;
      size++;
      werase(*type_win);
      mvwprintw(*type_win,0,0,message);
      wmove(*type_win,0,i);
      // waddch(*type_win, ch); //add it back to the window, but only if it isn't special
      //waddch(*type_win, ' ');
      wrefresh(*type_win); //refresh the window
    }
    //else if it's special
    switch (ch){ //switch so we can add diff stuff later
      int y, x;
    case KEY_BACKSPACE:
      deletechar(message,i);
      i--;
      size--;
      getyx(*type_win, y, x);
      //wmove(*type_win, y, x-1);
      mvwprintw(*type_win,0,0,message);
      wmove(*type_win,y,i);
      wrefresh(*type_win);
      break;
    case KEY_DC:
      i--;
      getyx(*type_win, y, x);
      wmove(*type_win, y, x-1);
      wrefresh(*type_win);
      break;
      /*
        case KEY_CH:
	i--;
	getyx(*type_win, y, x);
	wmove(*type_win, y, x-1);
	wrefresh(*type_win);
	break;
      */
    case KEY_UP:
      i--;
      getyx(*type_win, y, x);
      wmove(*type_win, y-1, x);
      //modify i to be the i of that point of the message, i think i = x * (y+1) not sure tho
      i = x * (y);
      wrefresh(*type_win);
      break;
    case KEY_DOWN:
      i--;
      getyx(*type_win, y, x);
      wmove(*type_win, y+1, x);
      i = x * (y+2);
      wrefresh(*type_win);
      break;
    case KEY_LEFT:
      i--;
      getyx(*type_win, y, x);
      wmove(*type_win, y, x-1);
      wrefresh(*type_win);
      break;
    case KEY_RIGHT:
      insertchar(message, i, ' ');
      i++;
      //size++;
      //instead, we should check in sending message whether or not i = size-1. if yes then we have to increment size
      getyx(*type_win, y, x);
      wmove(*type_win, y, x+1);
      wrefresh(*type_win);
      break;
    case KEY_F(1):
      wmove(*chat_win, 1, 1);
      wprintw(*chat_win, "f1 key\n");
      wrefresh(*chat_win); //refresh the window
      wmove(*type_win, 1, 1);
      return -1;
      break;
    case KEY_F(2):
      wmove(*game_win, 1, 1);
      wrefresh(*game_win);
      //actually some function should be called, called "interact with game" -- hilary's thing, it can have
      //a for loop identical to this and if F3 is called, go back to this
      //hm but this on it's own will return to typing bar if you press a character
      break;
    case KEY_F(3):
      wmove(*type_win, 0, 0);
      wrefresh(*type_win);
      break;
    case KEY_F(4):
      wmove(*chat_win, 1, 1);
      wrefresh(*chat_win);
      //some function should be called here where you can scroll thru the chat, and if F3 is called go back to this
      //hm but this on it's own will return to typing bar if you press a character
      break;
    default:
      break;
    }
  }else if (ch == '\n'){
    // initiate chat sending process
    //networking stuff
    werase(*type_win);
    insertchar(message,size,'\n');
    sendchat(message, size+1);
    // add_to_log(message, size+1); //we use i to see if write fails
    // print_log(chat_win);//print the log to the chat window
    memset(message,0,128);
    i = 0; //reset the message
    size = 0;
    wrefresh(*type_win); //move cursor back
  }else if (i == 126){
    // keep going until they press enter those fools
  }
  *ind = i;
  *sz = size;
  return 0;
}

int add_to_log(char *message, int i,int fd){
  //move to the end lseek(fd, )
  int check = write(fd, message, i);
  if (check != i){
    perror("addtolog");
    printf("fd of log: %d\n",fd);
    return 1;
  }
  return 0;
}

int print_log(WINDOW **log_window,int fd){
  //wmove(*chat_win, 1, 1);
  //wprintw(*chat_win, message);
  //wrefresh(*chat_win);
  werase(*log_window);
  wrefresh(*log_window);
  wmove(*log_window, 1, 0);
  int ret_read = 1; //end the while loop when read returns that it's read 0 bytes
  char buf; //to put on screen
  lseek(fd,0,SEEK_SET);
  while (ret_read){
    ret_read = read(fd, &buf, ret_read); //read in 1 char at a time
    waddch(*log_window, buf);
    wrefresh(*log_window);
  }
  return 0;
}
int setup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win){
  int startx, starty, width, height;
  printf("in setup\n");
  initscr();			/* Start curses mode 		*/
  cbreak();			/* Line buffering disabled, Pass on
				 * everty thing to me 		*/
  noecho(); //so that what you type doesn't show up on the screen
  keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
  //scrollok(*chat_win, TRUE);
  //scrollok(*type_win, TRUE);
  //our three boxes
  height = LINES - 2;
  width = COLS / 2;
  starty = 1;	/* Calculating for a center placement */
  startx = 1;	/* of the window		*/
  printw("F1: Exit, F2: Move avatar, F3: Type chat, F4: Scroll chat");
  //move cursor to middle and for height of screen draw vertical line
  int pos = width; //xcor
  int i = 1;
  while (i < LINES){
    mvaddch(i, pos, ACS_VLINE);
    i++;
  }
  //also draw horizontal line, ACS_HLINE --actually there should already be room at LINES - 5
  pos = LINES - 5; //ycor
  i = width; //so that it doesn't go into game window
  mvaddch(pos, i, ACS_LTEE);
  i++;
  while(i<COLS){
    mvaddch(pos, i, ACS_HLINE);
    i++;
  }
	refresh();
  //making room for the veritcal line
	*game_win = create_newwin(height, width-1, starty, startx);
  height = LINES - 6;
  width = COLS - (COLS / 2 + 3);
  starty = 1;
  startx = COLS / 2 + 2;
  *chat_win = create_newwin(height, width, starty, startx);
  height = 3;
  width = COLS - (COLS / 2 + 3);
  starty = LINES - 4;
  startx = COLS / 2 + 2;
  *type_win = create_newwin(height, width, starty, startx);
  //we don't need the box so let's erase
  //werase(*type_win);
  //DONT redraw the box
  wrefresh(*type_win);
  // // //don't need box for type

  return 0; //just to show that it works
}

int cleanup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win){
  destroy_win(*game_win);
  destroy_win(*chat_win);
  destroy_win(*type_win);
  return 0;
}

void destroy_win(WINDOW *local_win){
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners
	 * and so an ugly remnant of window.
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window
	 * 3. rs: character to be used for the right side of the window
	 * 4. ts: character to be used for the top side of the window
	 * 5. bs: character to be used for the bottom side of the window
	 * 6. tl: character to be used for the top left corner of the window
	 * 7. tr: character to be used for the top right corner of the window
	 * 8. bl: character to be used for the bottom left corner of the window
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}

void background(WINDOW **game_win, WINDOW **type_win) {
	attron(COLOR_PAIR(1));

  wmove(*game_win, LINES - 7, 0);
  whline(*game_win, ACS_HLINE, COLS / 2);
  /*
  int i = 0;
  while(i < COLS / 2){
    mvwaddch(*game_win, LINES - 7, i, ACS_HLINE);
    i++;
  }
  */
  //mvwprintw(*game_win, 2, 5, "A");
  wrefresh(*game_win);

  attroff(COLOR_PAIR(1));
}

void display_A(WINDOW **game_win, struct cnx_header *users) {
  //4: blue and blue
  //3: yellow and green
  //2: red and black
  //1: black and white
  int x,y;
  getyx(*game_win,y,x);
  /*
  wattron(*game_win, COLOR_PAIR(1));
  mvwprintw(*game_win, y, x, " ");
  wattroff(*game_win, COLOR_PAIR(1));
  */
  //Print grass
  wattron(*game_win, COLOR_PAIR(3));
  int b = 3;
  int a = 0;
  while (b <= LINES - 2) {
    while (a <= COLS / 2 - 1) {
      mvwprintw(*game_win, b, a, " ");
      a++;
    }
    a = 0;
    b++;
  }
  wattroff(*game_win, COLOR_PAIR(3));

  //Print sky
  wattron(*game_win, COLOR_PAIR(4));
  b = 0;
  a = 0;
  while (b <= 2) {
    while (a <= COLS / 2 - 1) {
      mvwprintw(*game_win, b, a, " ");
      a++;
    }
    a = 0;
    b++;
  }
  wattroff(*game_win, COLOR_PAIR(4));

  wattron(*game_win, COLOR_PAIR(2));
  int i;
  for(i = 0; i < MAX_CNX; i++ ){
    if( users[i].id >= 0 ){
      wmove(*game_win, users[i].pos.r , users[i].pos.c);
      waddch(*game_win, ACS_BLOCK);
    }
  }
  wattroff(*game_win, COLOR_PAIR(2));
  //mvwprintw(*game_win, y + y_move, x + x_move, "\U0001F427");
  wmove(*game_win,y,x);
  wrefresh(*game_win);
}

int in_gamewin(WINDOW *gamewin){
  int x,y;
  getyx(gamewin,x,y);
  return x==1 && y==1;
}
