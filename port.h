#ifndef PORT_H
#define PORT_H

#include "main.h"

#define COM_BAUD	256000
#define PORT_MAX_LEN	16

typedef struct {
	char serial[32];
	int version;
	int flash;
} taurus_info_t;

typedef void (*wait_callback)();

void port_open(void);

int port_cnt();
int port_list(char **names, char **descs, taurus_info_t **taurus);
int port_detect(char *name);
void port_wait_detect(char *name, wait_callback cb);
void port_list_free(int cnt, char **names, char **descs, taurus_info_t **taurus);

extern char port[PORT_MAX_LEN];
extern struct sp_port *serport;

#endif