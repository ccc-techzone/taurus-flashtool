#ifndef OPS_H
#define OPS_H

#define OP_NON		0
#define OP_FLASH	1
#define OP_READ		2
#define OP_ERASE	3

typedef void (*progress_callback_t)(int);

void op_connect();

void op_flash(char *file, progress_callback_t pcb);
void op_erase();

extern int operation;
extern int operations[3];

#endif