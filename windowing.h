#ifndef WINDOWING_H
#define WINDOWING_H

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
int setup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win);
int cleanup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win);
int read_from_type(WINDOW **type_win, WINDOW **print_errs,int *i,char *message);
int add_to_log(char *message, int i); //if file doesn't already exist, will create
int print_log(WINDOW **log_window); //clears, adds and reprints. must refresh typing window after to move cursor

#endif
