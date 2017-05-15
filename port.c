#include "port.h"

#include <stdio.h>
#include <libserialport.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

char port[PORT_MAX_LEN] = "";
struct sp_port *serport;

int port_cnt() {
	struct sp_port **ports;
	if (sp_list_ports(&ports) != SP_OK) {
		return 1;
	} else {
		int i = 0;
		for (i = 0; ports[i]; i++);
		sp_free_port_list(ports);
		return i + 1; // +1: debug port
	}
}

int port_list(char **names, char **descs, taurus_info_t **taurus) {
	struct sp_port **ports;
	if (sp_list_ports(&ports) != SP_OK) {
		return 0;
	} else {
		int i = 0;
		for (i = 0; ports[i]; i++) {
			int usb_vid = 0; sp_get_port_usb_vid_pid(ports[i], &usb_vid, NULL);
			char *product = sp_get_port_usb_product(ports[i]);
			char *manufacturer = sp_get_port_usb_manufacturer(ports[i]);
			char *port_name = sp_get_port_name(ports[i]);
			char *port_description = sp_get_port_description(ports[i]);
			char *serial = sp_get_port_usb_serial(ports[i]);

			int likely_taurus = 0;

			//if (usb_vid == 0x0483 && strstr(product, "TAURUS") != NULL && strstr(manufacturer, "CCC-TECHZONE.COM") != NULL)
			//	likely_taurus = 1;
			if (strcmp(port_name, "/dev/ttyACM0") == 0) {
				likely_taurus = 1;
			}

			names[i] = (char *) malloc(strlen(port_name) + 1);
			strcpy(names[i], port_name);
			descs[i] = (char *) malloc(strlen(port_description) + 1);
			strcpy(descs[i], port_description);

			if (likely_taurus) {
				char str_flash_size[5];
				strncpy(str_flash_size, serial + 3, 4);
				int flash_size = atoi(str_flash_size);

				taurus[i] = (taurus_info_t *) malloc(sizeof(taurus_info_t));
				taurus[i]->version = serial[1] - '0';
				taurus[i]->flash = flash_size;
				strncpy(taurus[i]->serial, serial, 31);
				taurus[i]->serial[31] = 0;
			} else {
				taurus[i] = NULL;
			}
		}
		sp_free_port_list(ports);

		names[i] = (char *) malloc(strlen("debug") + 1);
		descs[i] = (char *) malloc(strlen("no real io") + 1);
		strcpy(names[i], "debug");
		strcpy(descs[i], "no real io");
		taurus[i] = NULL;

		return i + 1;		// +1: debug port
	}
}

void port_list_free(int cnt, char **names, char **descs, taurus_info_t **taurus) {
	for (int i = 0; i < cnt; i++) {
		free(names[i]);
		free(descs[i]);
		free(taurus[i]);
	}
}

void open_port(void) {
	if (strcmp(port, "debug") == 0) {
		is_port_debug = 1;
		debug_printf("[INF] Debugging port!\n");
		return;
	}

	debug_printf("Opening port %s\n", port);

	enum sp_return err = sp_get_port_by_name(port, &serport);
	if (err != SP_OK) {
		debug_printf("[ERR] Error opening serial device 1: %s\n", sp_last_error_message());
		exit(EXIT_FAILURE);
	}

	err = sp_open(serport, SP_MODE_READ_WRITE);
	if (err != SP_OK) {
		debug_printf("[ERR] Error opening serial device 2: %s\n", sp_last_error_message());
		exit(EXIT_FAILURE);
	}

	sp_set_baudrate(serport, COM_BAUD);
}
