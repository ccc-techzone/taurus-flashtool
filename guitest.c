#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#define ASIZEOF(a)    (sizeof(a) / sizeof(a[0]))

#define MEN_OPERATION   0

WINDOW *win;
MENU *menu;

char *menu_title = "Test menu";
char *menu_choices[] = {
  "Option 1",
  "Option 2",
  "Option 3",
  "Option 4",
  (char *) NULL,
};

void quit(void) {
  delwin(win);
  endwin();
}

void init_curses(void) {
  initscr();
  atexit(quit);
  clear();
  noecho();
  curs_set(0);
  cbreak();
  keypad(stdscr, 1);

  start_color();

  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
}

void wtitle(WINDOW *win, const char *title) {
  int title_len = strlen(title);
  int maxx = 0, maxy = 0;
  getmaxyx(win,maxy,maxx);

  int title_x = (maxx - 4 - title_len) / 2;
  mvwaddch(win, 0, title_x, ACS_RTEE);
  waddch(win, ' ');
  waddstr(win, title);
  waddch(win, ' ');
  waddch(win, ACS_LTEE);

  wmove(win, 1, 1);
}

void change_menu(int menu_index) {
  int item_cnt = ASIZEOF(menu_choices);
  ITEM **items = (ITEM **) calloc(item_cnt, sizeof(ITEM *));
  for (int i = 0; i < item_cnt; i++)
    items[i] = new_item(menu_choices[i], "");

  menu = new_menu(items);
  set_menu_mark(menu, "--> ");

  int menu_width = 0;
  int menu_height = 0;
  scale_menu(menu, &menu_height, &menu_width);

  int win_x = 0, win_y = 0;
  getmaxyx(win, win_y, win_x);

  set_menu_win(menu, win);
  set_menu_sub(menu, derwin(win, win_y - 2, win_x - 2, 1, 1));

  refresh();
  post_menu(menu);
  wrefresh(win);
}

int main(void) {
  init_curses();

  int win_h = 10, win_w = 40;
  win = newwin(win_h, win_w, LINES / 2 - win_h / 2, COLS / 2 - win_w / 2);
  keypad(win, TRUE);
  box(win, 0, 0);
  wtitle(win, "Window");

  change_menu(0);

  WINDOW *statusbar = newwin(1, COLS, LINES - 1, 0);
  wbkgdset(statusbar, COLOR_PAIR(3));
  wprintw(statusbar, "F1 to cancel");

  refresh();
  wrefresh(statusbar);
  wrefresh(win);

  int c = 0;

  while((c = wgetch(win)) != KEY_F(1)) {
    switch(c) {
      case KEY_DOWN:
        menu_driver(menu, REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(menu, REQ_UP_ITEM);
        break;
    }
    wrefresh(win);
  } 


  unpost_menu(menu);
  free_menu(menu);


  return(0);
}
