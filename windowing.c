#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "windowing.h"
#include "client.h"

WINDOW *create_newwin(int height, int width, int starty, int startx){
  WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

int read_from_type(WINDOW **type_win, WINDOW **print_errs,int *ind,char *message){
  int i = *ind;
  int ch = wgetch(*type_win); //get what the user puts down
  if (i < 126 && ch != '\n'){
    switch (ch){ //switch so we can add diff stuff later
    case KEY_UP:
      wmove(*print_errs, 1, 1);
      wprintw(*print_errs, "key up");
          wrefresh(*print_errs); //refresh the window
          wmove(*type_win, 0, i+1);
          break;
    case KEY_DOWN:
      wmove(*print_errs, 1, 1);
      wprintw(*print_errs, "key down");
      wrefresh(*print_errs); //refresh the window
      wmove(*type_win, 0, i+1);
      break;
    case KEY_LEFT:
      wmove(*print_errs, 1, 1);
      wprintw(*print_errs, "key left");
      wrefresh(*print_errs); //refresh the window
      wmove(*type_win, 0, i+1);
      break;
    case KEY_RIGHT:
      wmove(*print_errs, 1, 1);
      wprintw(*print_errs, "key right");
      wrefresh(*print_errs); //refresh the window
      wmove(*type_win, 0, i+1);
      break;
    case KEY_F(1):
      wmove(*print_errs, 1, 1);
      wprintw(*print_errs, "f1 key\n");
      wrefresh(*print_errs); //refresh the window
      wmove(*type_win, 1, 1);
      return 0; //end the function
      break;
    default:
      message[i] = ch;
      i++;
      waddch(*type_win, ch); //add it back to the window, but only if it isn't special
      //waddch(*type_win, ' ');
      wrefresh(*type_win); //refresh the window
      break;
    }
  }else if (ch == '\n'){
    // initiate chat sending process
    //networking stuff
    werase(*type_win);
    //DONT redraw the box
    //just for checking, print message
    //wmove(*print_errs, 1, 1);
    //wprintw(*print_errs, message);
    //wrefresh(*print_errs);
    //and clear it
    message[i] = ch; //add new line to message
    // add_to_log(message, i+1); //we use i to see if write fails
    sendchat(message);
    
    // print_log(print_errs);//print the log to the chat window
    message[i] = ' ';
    i = 0; //reset the message
    wrefresh(*type_win); //move cursor back
  }else if (i == 126){
    // keep going until they press enter those fools
  } //if you press f1, this returns and main goes onto cleanup and end
  *ind = i;
  return 0;
}

int setup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win){
	int startx, starty, width, height;

	initscr();			/* Start curses mode 		*/
	cbreak();			/* Line buffering disabled, Pass on
					 * everty thing to me 		*/
  noecho(); //so that what you type doesn't show up on the screen
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/

  //our three boxes
	height = LINES - 2;
	width = COLS / 2;
	starty = 1;	/* Calculating for a center placement */
	startx = 1;	/* of the window		*/
	printw("Press F1 to exit");
	refresh();
	*game_win = create_newwin(height, width, starty, startx);
  height = LINES - 5;
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
  werase(*type_win);
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
