#include "gui.h"
#include "port.h"
#include "ops.h"

#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


static void quit(void);

WINDOW *win;
WINDOW *debug_frame;
WINDOW *debug;
WINDOW *received;

void quit(void) {
	int win_h = 11, win_w = 40;
	WINDOW *closewin = newwin(win_h, win_w, LINES / 2 - win_h / 2, COLS / 2 - win_w / 2);
	wbkgd(closewin, COLOR_PAIR(CP_RED_WHITE));
	box(closewin, 0, 0);

	mvwprintw(closewin, 5, 9, "Press any key to exit");


	refresh();
	wrefresh(closewin);

	gui_wait();

	delwin(win);
	endwin();
}

void mvwtitlew(WINDOW *win, int y, int x, const char *title, int width) {
	int tx = 0, ty = 0;
	int title_len = strlen(title);
	getyx(win, ty, tx);

	int title_x = (width - 4 - title_len) / 2;
	mvwaddch(win, y, title_x + x, ACS_RTEE);
	waddch(win, ' ');
	waddstr(win, title);
	waddch(win, ' ');
	waddch(win, ACS_LTEE);

	wmove(win, ty, tx);
}

void wtitle(WINDOW *win, const char *title) {
	int maxx = 0, maxy = 0;
	getmaxyx(win,maxy,maxx);

	//box(win, 0, 0);

	mvwtitlew(win, 0, 0, title, maxx);
}

void gui_init(void) {
	initscr();
	atexit(quit);
	clear();
	noecho();
	curs_set(0);
	cbreak();
	keypad(stdscr, 1);

	start_color();

	init_color(10, 500, 500, 500);

	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);
	init_pair(CP_GRAY_WHITE, COLOR_WHITE, 10);
	init_pair(CP_RED_WHITE, COLOR_WHITE, COLOR_RED);
	init_pair(CP_PROGRESSBAR, COLOR_WHITE, COLOR_GREEN);

	debug_frame = newwin(LINES / 3 + 1, COLS, 2 * LINES / 3, 0);
	wbkgd(debug_frame, COLOR_PAIR(CP_GRAY_WHITE));
	//scrollok(debug_frame, TRUE);
	whline(debug_frame, 0, COLS);
	mvwtitlew(debug_frame, 0, 0, "Debug", COLS / 2);
	mvwtitlew(debug_frame, 0, COLS / 2, "Received", COLS / 2);
	mvwaddch(debug_frame, 0, COLS / 2, ACS_TTEE);
	mvwvline(debug_frame, 1, COLS / 2, ACS_VLINE, LINES / 3);

	debug = derwin(debug_frame, LINES / 3 - 1, COLS / 2, 1, 0);
	scrollok(debug, TRUE);
	wmove(debug, 1, 0);

	received = derwin(debug_frame, LINES / 3 - 1, COLS / 2 - 1, 1, COLS / 2 + 1);
	scrollok(received, TRUE);
	//wbkgd(received, COLOR_PAIR(CP_RED_WHITE));
	wmove(received, 0, 0);
}

void gui_wait(void) {
	getch();
}

void guiRefresh(void) {
	refresh();
	wrefresh(win);
	wrefresh(debug);
	wrefresh(received);
	wrefresh(debug_frame);
}

void flash_win(int size, int packets) {
	int win_h = 10, win_w = 60;
	win = newwin(win_h, win_w, LINES / 3 - win_h / 2, COLS / 2 - win_w / 2);
	keypad(win, TRUE);
	box(win, 0, 0);
	wtitle(win, "Flash");
	mvwprintw(win, 1, 1, "Packets : %i", packets);
	mvwprintw(win, 2, 1, "Size    : %.3fkB", size / 1024.0);
}

void flash_progress(int progr) {
	int maxx = 0, maxy = 0, offs_x = 3;;
	getmaxyx(win, maxy, maxx);
	int cnt = (maxx - 2 * offs_x) * progr / 100;

	for (int i = 0; i < cnt; i++) {
		wmove(win, 5, offs_x + i);
		wattron(win, COLOR_PAIR(CP_PROGRESSBAR));
		waddch(win, ' ');
		wattroff(win, COLOR_PAIR(CP_PROGRESSBAR));
	}
}

void debug_printf(const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);

	if (strstr(fmt, "ERR") != NULL) {
		wcolor_set(debug, CP_RED_WHITE, 0);
	}

	vwprintw(debug, fmt, argp);

	wcolor_set(debug, 0, 0);

	va_end(argp);

	guiRefresh();
}

void print_hex(uint8_t *data, int len) {
	for (int i = 0; i < len; i++) {
		debug_printf("0x%02x ", data[i]);
	}
	debug_printf("\n");
}



///////////////////////////////////////////////////////////////////////////////////



int gui_query_op() {
	char *choices[] = {
		"Flash",
		"Read",
		"Erase",
		NULL
	};
	ITEM **items = (ITEM **)calloc(4, sizeof(ITEM *));

	for(int i = 0; i < 4; i++) {
	    items[i] = new_item(choices[i], "");
	    //set_item_userptr(items[i], ti[i]);
	}

	MENU *op_menu = new_menu((ITEM **) items);
    set_menu_mark(op_menu, "--> ");

    int menu_maxx = 0, menu_maxy = 0;
    scale_menu(op_menu, &menu_maxy, &menu_maxx);

    // create menu window
    delwin(win);
	int win_h = menu_maxy + 4, win_w = 30;
	win = newwin(win_h, win_w, LINES / 3 - win_h / 2, COLS / 2 - win_w / 2);
	keypad(win, TRUE);
	box(win, 0, 0);
	wtitle(win, "Select operation");

	set_menu_win(op_menu, win);
    set_menu_sub(op_menu, derwin(win, menu_maxy, menu_maxx, 2, 1));
	post_menu(op_menu);

	guiRefresh();

	int c;
	while((c = wgetch(win)) != '\n') {
	    switch(c) {
	    	case KEY_DOWN:
				menu_driver(op_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(op_menu, REQ_UP_ITEM);
				break;
		}
	}

	int op_i = item_index(current_item(op_menu));
	int op = operations[op_i];

	unpost_menu(op_menu);
	free_menu(op_menu);
	for(int i = 0; i < 4; ++i)
		free_item(items[i]);

	delwin(win);
	werase(win);

	guiRefresh();

	return op;
}

void gui_query_port(char *port, int len) {
	int pcnt = port_cnt();
	ITEM **items = (ITEM **)calloc(pcnt , sizeof(ITEM *));
	char **names = (char **) malloc(pcnt * sizeof(char *));
	char **descs = (char **) malloc(pcnt * sizeof(char *));
	taurus_info_t **ti = (taurus_info_t **) malloc((pcnt + 1) * sizeof(taurus_info_t *));

	port_list(names, descs, ti);

	for(int i = 0; i < pcnt; i++) {
	    items[i] = new_item(names[i], descs[i]);
	    set_item_userptr(items[i], ti[i]);
	}
	items[pcnt] = (ITEM *) NULL;

	MENU *port_menu = new_menu((ITEM **) items);
    set_menu_mark(port_menu, "--> ");

    int menu_maxx = 0, menu_maxy = 0;
    scale_menu(port_menu, &menu_maxy, &menu_maxx);
    if (menu_maxx < strlen("Select Taurus for details")) {
    	menu_maxx = strlen("Select Taurus for details") + 2;
    }

    // create menu window
    delwin(win);
	int win_h = menu_maxy + 10, win_w = menu_maxx + 4;
	win = newwin(win_h, win_w, LINES / 3 - win_h / 2, COLS / 2 - win_w / 2);
	keypad(win, TRUE);
	box(win, 0, 0);
	wtitle(win, "Select serial port");
	mvwprintw(win, 1, 1, "Select Taurus for details");

	set_menu_win(port_menu, win);
    set_menu_sub(port_menu, derwin(win, menu_maxy, menu_maxx, 3, 2));
	post_menu(port_menu);

	WINDOW *details = derwin(win, 4, win_w - 2, menu_maxy + 5, 1);
	wattron(details, A_BOLD);
	mvwaddch(win, menu_maxy + 4, 0, ACS_LTEE);
	mvwhline(win, menu_maxy + 4, 1, ACS_HLINE, win_w - 2);
	mvwaddch(win, menu_maxy + 4, win_w - 1, ACS_RTEE);

	int c;
	taurus_info_t *t_info = item_userptr(current_item(port_menu));
	debug_printf("t_info: %p\n", t_info);
	if (t_info != NULL) {
		//wbkgd(details, COLOR_PAIR(CP_PROGRESSBAR));
		mvwprintw(details, 0, 0, "Version: %i | Flash: %ikB", t_info->version, t_info->flash);
	} else {
		//wbkgd(details, COLOR_PAIR(CP_RED_WHITE));
		wclear(details);
	}

	guiRefresh();
	wrefresh(details);

	while((c = wgetch(win)) != '\n') {
	    switch(c) {
	    	case KEY_DOWN:
				menu_driver(port_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(port_menu, REQ_UP_ITEM);
				break;
		}

		t_info = item_userptr(current_item(port_menu));
		debug_printf("t_info: %p\n", t_info);
		if (t_info != NULL) {
			//wbkgd(details, COLOR_PAIR(CP_PROGRESSBAR));
			mvwprintw(details, 0, 0, "Version: %i | Flash: %ikB", t_info->version, t_info->flash);
		} else {
			//wbkgd(details, COLOR_PAIR(CP_RED_WHITE));
			wclear(details);
		}

        guiRefresh();
        wrefresh(details);
	}

	strncpy(port, item_name(current_item(port_menu)), len);
	debug_printf("Port1: %s\n", port);
	guiRefresh();

	unpost_menu(port_menu);
	free_menu(port_menu);
	for(int i = 0; i < pcnt - 1; i++)
		free_item(items[i]);
	port_list_free(pcnt, names, descs, ti);

	delwin(details);
	delwin(win);
	werase(win);

	guiRefresh();

	return;
}

void gui_progress(int p) {
	int maxx = 0, maxy = 0, offs_x = 3;;
	getmaxyx(win, maxy, maxx);
	int cnt = (maxx - 2 * offs_x) * p / 100;

	for (int i = 0; i < cnt; i++) {
		wmove(win, 5, offs_x + i);
		wattron(win, COLOR_PAIR(CP_PROGRESSBAR));
		waddch(win, ' ');
		wattroff(win, COLOR_PAIR(CP_PROGRESSBAR));
	}

	refresh();
	wrefresh(win);
}

void gui_received(uint8_t *buf, size_t len) {
	debug_printf("Received %i bytes\n", len);

	for (int i = 0; i < len; i++) {
		if (buf[i] == '\r') {
			buf[i] = ' ';
 		}
		waddch(received, buf[i]);
	}
	refresh();
	wrefresh(received);
}