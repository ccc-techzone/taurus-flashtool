#ifndef MAIN_H
#define MAIN_H


void display_help(char **argv);
void listports(void);
void open_port(void);
void readOptions(int argc, char **argv);

extern int is_port_debug;

#endif