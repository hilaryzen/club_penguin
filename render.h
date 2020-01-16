#ifndef RENDER_H
#define RENDER_H

#define PENGUIN "\U0001F427"
#define CLEAR_TERMINAL "\e[2J\e[0;0f"
#define ROWCOUNT 150
#define COLCOUNT 150

void resize_term();
void draw_penguin(int r,int c);
void print_term();
#endif
