#ifndef WINDOWS_H
#define WINDOWS_H
#include "packets.h"

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
int setup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win);
int cleanup(WINDOW **game_win, WINDOW **chat_win, WINDOW **type_win);
int add_to_log(char *message, int i,int fd);
int read_from_type(WINDOW **type_win, WINDOW **print_errs, WINDOW **game_win,char *msg,int *i,int *size);
int print_log(WINDOW **log_window,int fd); //clears, adds and reprints. must refresh typing window after to moxve cursor

void background(WINDOW **game_win, WINDOW **type_win);
void display_A(WINDOW **game_win, WINDOW **type_win, int y, int x, int y_move, int x_move);

int arrow_game(WINDOW **game_win, WINDOW **type_win, struct playermove *me);
int in_gamewin(WINDOW *game_win);
#endif
