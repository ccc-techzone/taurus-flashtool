#include "ops.h"
#include "port.h"
#include "gui.h"

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <libserialport.h>

int operations[3] = {OP_FLASH, OP_READ, OP_ERASE};
int operation = OP_NON;

void op_flash(char *file, progress_callback_t pcb) {
	int fd;
	struct stat sb;
	char *data;

	fd = open(file, O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (fstat(fd, &sb) == -1) {
		perror("fstat");
		exit(EXIT_FAILURE);
	}

	data = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}


	// window size for progress bar
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);

	int leftover = sb.st_size;
	int packet_size = 4;
	int total_packets = leftover / packet_size;
	int packet_counter = 0;
	int packets_per_dot = total_packets / (w.ws_col - 10);

	flash_win(sb.st_size, total_packets);

	uint16_t len = sb.st_size;
	uint8_t len_frame[] = {len & 0xFF, len >> 8};
	//uint8_t len_frame[] = {0x16, 0x32};

	print_hex(len_frame, 2);

	// 1234AABBCC1234
	char idbuf[14];
	int count = 0;

	while (count < 5 && !is_port_debug) {
		if (sp_blocking_read(serport, idbuf, 14, 0) < 0) {
			count = 5;
			break;
		}
		debug_printf("Received: %.14s\n", idbuf);
		if (memcmp(idbuf, "1234AABBCC1234", 14) == 0) {
			debug_printf("Received ID!\n");
			char idAns[] = "4321CCBBAA4321";
			sp_blocking_write(serport, idAns, 14, 0);
			break;
		}
		sleep(1);
		count++;
	}
	if (count == 5) {
		debug_printf("[ERR] ID Error!\n");
		return;
	}

	while (sp_input_waiting(serport)) {
		char c;
		sp_blocking_read(serport, &c, 1, 0);
		gui_received(&c, 1);
	}

	//char op[] = {0xA5, 0xF1};
	char op[] = {0xA5, 0xE2};
	sp_blocking_write(serport, op, 2, 0);
	return;


	if (!is_port_debug && sp_blocking_write(serport, len_frame, 2, 0) != 2) {
		exit(EXIT_FAILURE);
	}

	while (leftover > 0) {
		int size = leftover < packet_size ? leftover : packet_size;
		uint8_t packet[packet_size];
		memset(packet, 0xFF, packet_size);
		memcpy(packet, data + sb.st_size - leftover, size);

		if (!is_port_debug && sp_blocking_write(serport, packet, packet_size, 0) != packet_size) {
			exit(EXIT_FAILURE);
		}

		print_hex(packet, packet_size);
		//flash_progress(packet_counter * 100 / total_packets);
		pcb(packet_counter * 100 / total_packets);

		leftover -= packet_size;
		packet_counter++;

		//usleep(500);

		if (sp_input_waiting(serport)) {
			uint8_t buf[128];
			int len = sp_nonblocking_read(serport, buf, 128);

			if (len > 0) {
				debug_printf("Read %i bytes\n", len);
				gui_received(buf, len);
			}
		}

		debug_printf("i: %i | leftover: %i\n", packet_counter, leftover);

		//sleep(5);
	}

	debug_printf("Left: %i\n", leftover);

	munmap(data, sb.st_size);
	close(fd);
}
