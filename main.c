#include <stdio.h>
#include <getopt.h>    /* for getopt_long; POSIX standard getopt is in unistd.h */
#include <string.h>
#include <stdlib.h>

#include <libserialport.h>

#include "main.h"
#include "gui.h"
#include "ops.h"
#include "port.h"


int is_port_debug = 0;
int is_interactive = TRUE;
char *file = NULL;

static struct option options[] = {
	{"flash", 0, 0, 0},
	{"read", 0, 0, 0},
	{"erase", 0, 0, 0},
	{"port", 1, 0, 0},
	{"help", 0, 0, 0},
	{NULL, 0, NULL, 0}
};

void display_help(char **argv) {
	printf("Usage: %s [--flash,--read,--erase] --port [port] {options} {file}\n\n", argv[0]);
	printf("\t--flash\t\tFlashes {file}\n"
	                "\t--read\t\tReads {file}\n"
	                "\t--erase\t\tErase the flash\n\n");
	printf("Options:\n"
	                "\t--port [port]\tselects the serial port to use (? to list all)\n"
	                "\t--help\t\tdisplays this help message\n");
	
}

void listports() {
	int pcnt = port_cnt();
	char **names = (char **) malloc(pcnt * sizeof(char *));
	char **descs = (char **) malloc(pcnt * sizeof(char *));
	taurus_info_t **ti = (taurus_info_t **) malloc(pcnt * sizeof(taurus_info_t *));

	port_list(names, descs, ti);

	for (int i = 0; i < pcnt; i++) {
		if (ti[i] != NULL) {
			printf("--> ");
		}
		printf("\t%s -- %s\t", names[i], descs[i]);

		if (ti[i] != NULL) {
			printf("[Version: %i | Size: %i]\t<--", ti[i]->version, ti[i]->flash);
		}
		printf("\n");
	}

	port_list_free(pcnt, names, descs, ti);

}

void readOptions(int argc, char **argv) {
	int c = 0;
	int option_index = 0;

	while ((c = getopt_long(argc, argv, "frep:hq",
		options, &option_index)) != -1) {
		switch (c) {
		case 0:
			switch (option_index) {
			case 0:
			case 1:
			case 2:
				if (operation == OP_NON) {
					operation = operations[option_index];
				} else {
					printf("Invalid combination of operations!\n");
					break;
				}
				break;
			case 3:
				if (optarg[0] == '?') {
					listports();
					exit(0);
				} else
					strncpy(port, optarg, PORT_MAX_LEN);
				break;
 			case 4:
				display_help(argv);
				break;
			}

			break;
		case 'q':
			// quiet
			is_interactive = 0;
			break;
		}
	}

	if (operation == OP_FLASH && optind == argc) {
	 	printf("Must specify file!\n\n");
	 	display_help(argv);
	 	exit(EXIT_FAILURE);
	}
	file = argv[optind];
}

void gui_main(int argc, char **argv) {
	gui_init();
	
	if (operation == OP_NON) {
		operation = gui_query_op();
		debug_printf("Op: %i\n", operation);
	}
	if (port[0] == '\0') {
		gui_query_port(port, PORT_MAX_LEN);
		debug_printf("Port: %s\n", port);
	}

	//gui_wait();
}


// ID
// FLASH: 0xF1A5 // ERASE: 0xE2A5
// LEN
// DATA


int main (int argc, char **argv) {
	readOptions(argc, argv);

	gui_init();

	port_wait_detect(port, gui_waiting);
	gui_reset();
	port_open();

	op_connect();

	if (operation == OP_NON) {
		operation = gui_query_op();
		debug_printf("Op: %i\n", operation);
	}

	switch (operation) {
	case OP_FLASH:
		op_flash(file, gui_progress);
		break;
	case OP_ERASE:
		op_erase();
		break;
	}

	// progress_callback_t pcb = NULL;

	// if(is_interactive) {
	// 	gui_main(argc, argv);
	// 	pcb = gui_progress;
	// } else {
	// 	tui_main(argc, argv);
	// 	pcb = tui_progress;
	// }

	// open_port();

	// op_flash(file, pcb);
	
	while (1) {
		if (sp_input_waiting(serport)) {
			uint8_t buf[128];
			int len = sp_nonblocking_read(serport, buf, 128);

			debug_printf("Read %i bytes\n", len);
			if (len > 0)
				gui_received(buf, len);
		}
	}

	// exit (0);
}


