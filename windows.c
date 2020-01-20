#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "windows.h"
#include "client.h"
#include "err.h"
//PATH = "log.txt"

void insertchar(char *buf,int i,char c){
  if (i == strlen(buf)){
    //so 'abc\0', if i is three just add to end 'abci\0'
    //even if it's j an empty message, it will be '\0' which has len zero so still caught by this
    buf[i] = c;
    buf[i+1] = '\0';
  }else{
    /*
    int j = strlen(buf);
    buf[j+1] = '\0';
    //say i is 2 and you have 'abcd\0' --> now it's 'abcd\0\0'
    //j = 4 and i is 2 so
    //j4,i2: 'abcdd\0' --> j=3
    //j3, i2: 'abccd\0' --> j=2
    while(j > i){
      buf[j] = buf[--j];
    }
    */
    //a dif way to do this: memmove()
    //'abcdefg\0', want to instert char at i = 3. then we move from 3-strlen, inclusive, to 4. then add to 3
    //now j2, i2, so: 'abCcd\0'
    //how much you're copying is = to (strlen - i) + 1, to account for the null
    //where i learned abt this: https://viewsourcecode.org/snaptoken/kilo/05.aTextEditor.html
    memmove(&buf[i+1], &buf[i], (strlen(buf) - i)+1);
    buf[i] = c;
  }
  //i needs to be incremented after this, so the cursor remain behind what it was originally behind
  //size also must be incremented
  //this functionality should be like you can only add behind something,
  //and if you want to delete it, move the cursor infront and then hit backspace
}

void deletechar(char *buf,int i){
  if (i == strlen(buf)){
    //don't delete the last null!
    //just don't do anything
    //there should also be a catch in backspacing that prevents
    //decrementing i past zero
  }else{
    //'abcd\0', say i = 2, we want 'abd\0'.
    //i = 2, 'abcd\0'-->'abdd\0', now i =3
    //i = 3, 'abdd\0' --> 'abd/0/0'
    //the length of our buffer should update to just 3 so it should stop here
    int j = strlen(buf);
    while(i<j){
      buf[i] = buf[++i];
    }
    buf[j-1] = '\0'; //i hope this works
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

int read_from_type(WINDOW **type_win, WINDOW **chat_win, WINDOW **game_win,char *message, int *ind,int *sz){
  int length_of_type = COLS - (COLS / 2 + 3);
  int i = *ind;
  int size = *sz;
  //remember we are catching special keys, called keypad(win, TRUE) in setup
  int ch = wgetch(*type_win); //get what the user puts down
  if (size < 126 && ch != '\n'){
    int y, x;
    if (!has_key(ch)){
      //
      insertchar(message, i, ch);
      i++;
      size++;
      //
      getyx(*type_win, y, x);
      //
      werase(*type_win);
      mvwprintw(*type_win,0,0,message);
      wmove(*type_win, 0, i);
      //
      //
      wrefresh(*type_win); //refresh the window
    }
    //else if it's special
    switch (ch){ //switch so we can add diff stuff later
    case KEY_BACKSPACE:
      i--;
      size--;
      deletechar(message, i); //you want to delete what is behind the cursor, and take it's place
      //
      //
      getyx(*type_win, y, x);
      werase(*type_win);
      mvwprintw(*type_win,0,0,message);
      wmove(*type_win,0, i);
      wrefresh(*type_win);
      break;
    case KEY_DC:
      i--;
      size--;
      deletechar(message, i); //you want to delete what is behind the cursor, and take it's place
      //
      //
      getyx(*type_win, y, x);
      werase(*type_win);
      mvwprintw(*type_win,0,0,message);
      wmove(*type_win,0, i);
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
      //
      //
      if (i > length_of_type){
        i = i-length_of_type;
        //getyx(*type_win, y, x);
        wmove(*type_win, 0, i);

        //
        wrefresh(*type_win);
      }
      break;
    case KEY_DOWN:
      //
      //
      if (size >= (i+length_of_type-1)){ //then there is a character to latch on to
        i = i+length_of_type;
        //getyx(*type_win, y, x);
        wmove(*type_win, 0, i);
        //
        //
        wrefresh(*type_win);
      }
      break;
    case KEY_LEFT:
      //just move the cursor, and where you are on the message, but don't affect anything
      //
      //i--;
      getyx(*type_win, y, x);
      if (i%length_of_type == 0){
        //means that in 'abcd\nefg', you're at n, have to decrease i and move up to (y-1, length_of_type)
        wmove(*type_win, y-1, length_of_type);
      }else{
        //just move over
        wmove(*type_win, y, x-1);
      }
      //wmove(*type_win, 0, i);
      wrefresh(*type_win);
      i--;
      break;
    case KEY_RIGHT:
      //insertchar(message, i, ' ');
      //FOR NOW, DON'T ALLOW I TO INCREMENT PAST SIZE!
      //
      //size++;
      //instead, we should check in sending message whether or not i = size-1. if yes then we have to increment size
      if (i == size){
        //can't do anything
      }else{
        //i++;
        getyx(*type_win, y, x);
        if (i%length_of_type == (length_of_type - 1)){
          //means that in 'abcd\nefg', you're at d, have to increase i and move to (y+1, 0)
          wmove(*type_win, y+1, 0);
        }else{
          //just move over
          wmove(*type_win, y, x+1);
        }
        //wmove(*type_win, 0, i);
        wrefresh(*type_win);
        i++;
      }
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
    insertchar(message, size, '\n');
    size++;
    //
    //
    sendchat(message, size);
    size = 0;
    i = 0;
    // add_to_log(message, size+1); //we use i to see if write fails
    // print_log(chat_win);//print the log to the chat window
    memset(message,0,128);
    message[0] = '\0';
    //
    //
    wrefresh(*type_win); //move cursor back
  }else if (size == 126){
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
  width = COLS - (COLS / 2 + 3); //this is how many characters long our box actually is?
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
