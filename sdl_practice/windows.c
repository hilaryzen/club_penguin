#include <ncurses.h>


WINDOW *create_newwin(int height, int width, int starty, int startx);
void display_A(WINDOW *game_win, WINDOW *type_win, int y, int x, int y_move, int x_move);

int main(int argc, char *argv[])
{	WINDOW *game_win;
  WINDOW *chat_win;
  WINDOW *type_win;
	int startx, starty, width, height;
	int ch;

	initscr();			/* Start curses mode 		*/
	cbreak();			/* Line buffering disabled, Pass on
					 * everty thing to me 		*/
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/

	height = LINES - 2;
	width = COLS / 2;
	starty = 1;	/* Calculating for a center placement */
	startx = 1;	/* of the window		*/
	printw("Press Escape to switch between windows and F1 to exit");
	refresh();
	game_win = create_newwin(height, width, starty, startx);
  height = LINES - 5;
  width = COLS - (COLS / 2 + 3);
  starty = 1;
  startx = COLS / 2 + 2;
  chat_win = create_newwin(height, width, starty, startx);
  height = 3;
  width = COLS - (COLS / 2 + 3);
  starty = LINES - 4;
  startx = COLS / 2 + 2;
  type_win = create_newwin(height, width, starty, startx);
  int y = LINES / 2; //Position of A
  int x = COLS / 4;
  display_A(game_win, type_win, y, x, 0, 0);
  /*
  wmove(game_win, 1, 1); //Moves cursor to game window
  mvwprintw(game_win, (LINES / 2), (COLS / 4), "A"); //Prints A in the middle of the game window
  wmove(type_win, 1, 1); //Moves cursor back to type window
  wrefresh(game_win);
  wrefresh(type_win);
  */


	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	case KEY_LEFT:
        display_A(game_win, type_win, y, x, 0, -1);
        x--;
				break;
			case KEY_RIGHT:
        display_A(game_win, type_win, y, x, 0, 1);
        x++;
				break;
			case KEY_UP:
        display_A(game_win, type_win, y, x, -1, 0);
        y--;
        break;
			case KEY_DOWN:
        display_A(game_win, type_win, y, x, 1, 0);
        y++;
				break;
      case 27: //Escape key
        wmove(game_win, 1, 1);
        wrefresh(game_win);
        break;
		}
	}

	endwin();			/* End curses mode		  */
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void display_A(WINDOW *game_win, WINDOW *type_win, int y, int x, int y_move, int x_move) {
  mvwprintw(game_win, y, x, " "); //Prints A in the middle of the game window
  mvwprintw(game_win, y + y_move, x + x_move, "A");
  wmove(type_win, 1, 1); //Moves cursor back to type window
  wrefresh(game_win);
  wrefresh(type_win);
}
