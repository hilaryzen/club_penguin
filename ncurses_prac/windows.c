#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


//PATH = "log.txt"

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
int setup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win);
int cleanup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win);
int read_from_type(WINDOW **type_win, WINDOW **print_errs);
int add_to_log(char *message, int i); //if file doesn't already exist, will create
int print_log(WINDOW **log_window); //clears, adds and reprints. must refresh typing window after to move cursor

int main(int argc, char *argv[]){
  WINDOW *game_win;
  WINDOW *chat_win;
  WINDOW *type_win;
  int ch;
  if (setup(&game_win, &chat_win, &type_win)){
    printf("uh oh, setup failed\n");
  }
  /*
	while((ch = getch()) != KEY_F(1))
	{
    wmove(type_win, 1, 1);
    wrefresh(type_win);
	}
  */
  if (read_from_type(&type_win, &chat_win)){
    printf("uh oh, can't read from type window!\n");
  }
  if (cleanup(&game_win, &chat_win, &type_win)){
    printf("uh oh, cleanup failed\n");
  }
  //sleep(2); just did this to test that cleanup works
	endwin();			/* End curses mode		  */
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx){
  WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

int read_from_type(WINDOW **type_win, WINDOW **print_errs){
  //remember we are catching special keys, called keypad(win, TRUE) in setup
  int ch = ' ';
  keypad(*type_win, TRUE);
  scrollok(*type_win, TRUE); //so if we've printed out of the window, will just scroll down
  scrollok(*print_errs, TRUE);
  wmove(*type_win, 0, 0); //set cursor
  char message[128];
  int i = 0;
  while(1){
    ch = wgetch(*type_win); //get what the user puts down
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
      add_to_log(message, i+1); //we use i to see if write fails
      print_log(print_errs);//print the log to the chat window
      message[i] = ' ';
      i = 0; //reset the message
      wrefresh(*type_win); //move cursor back
    }else if (i == 126){
      // keep going until they press enter those fools
    }
  } //if you press f1, this returns and main goes onto cleanup and end
  wmove(*print_errs, 1, 1);
  wprintw(*print_errs, "returning 0, broke out of loop somehow\n");
  return 0;
}

int add_to_log(char *message, int i){
  int fd = open("log.txt", O_WRONLY | O_CREAT | O_EXCL | O_APPEND, 0666);
  if (fd == -1){
    fd = open("log.txt", O_WRONLY | O_APPEND); //if it already exists, open w/o create
  }
  int check = write(fd, message, i);
  if (check != i){
    printf("not sure where on screen this is, but add_to_log not working\n");
    return 1;
  }
  close(fd);
  return 0;
}
int print_log(WINDOW **log_window){
  //wmove(*print_errs, 1, 1);
  //wprintw(*print_errs, message);
  //wrefresh(*print_errs);
  werase(*log_window);
  wrefresh(*log_window);
  wmove(*log_window, 1, 1);
  int fd = open("log.txt", O_RDONLY);
  if (fd == -1){
    printf("can't open log.txt in print_log\n");
  }
  int ret_read = 1; //end the while loop when read returns that it's read 0 bytes
  char buf; //to put on screen
  while (ret_read){
    ret_read = read(fd, &buf, ret_read); //read in 1 char at a time
    waddch(*log_window, buf);
    wrefresh(*log_window);
  }
  close(fd); //close when done
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
