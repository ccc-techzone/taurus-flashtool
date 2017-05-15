#ifndef MAIN_H
#define MAIN_H

#define OP_NON		0
#define OP_FLASH	1
#define OP_READ		2
#define OP_ERASE	3

#define COM_BAUD	9600

#define PORT_MAX_LEN	16

void display_help(char **argv);
void listports(void);
void open_port(void);
void op_flash(char *file);
void readOptions(int argc, char **argv);



#endif