#ifndef GUI_H
#define GUI_H

#include <curses.h>

#define COLOR_GRAY		10

#define CP_GRAY_WHITE	4
#define CP_RED_WHITE	5
#define CP_PROGRESSBAR	6
#define CP_ERROR		7


void gui_init(void);
void gui_reset();
void gui_wait(void);
void gui_refresh(void);
void wtitle(WINDOW *win, const char *title);

void flash_win(int size, int packets);
void flash_progress(int progr);

void debug_printf(const char *fmt, ...);
void print_hex(uint8_t *data, int len);


int gui_query_op();
void gui_query_port(char *port, int len);
void gui_progress(int p);
void gui_waiting();
void gui_received(uint8_t *buf, size_t len);

int gui_redo();



extern WINDOW *win;
extern WINDOW *debug;
extern int proper_exit;

#endif