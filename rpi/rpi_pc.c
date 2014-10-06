/*
 * port_no: 5000
 * ip: 192.168.22.1
 * baud rate: 115200
 * serial port: "/dev/ttyACM0"
 * bluetooth UUID: {0x1101, 0x1000, 0x80000080 ,0x5f9b34fb}
 * to compile, type in terminal: gcc -o rpi rpi.c ip.c rfcomm.c serial.c -lpthread -lbluetooth -lwiringPi
 * to run rpi, type in terminal: ./rpi 5000
 * one problem: if message from arduino is split using "|" then rpi will process it multiple times.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <error.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include "global.h"
#include "ip.h"
#include "rfcomm.h"
#include "serial.h"

#define STDIN 0
#define PORT_NO 5000
#define BAUD 115200
#define DEVICE_ARDUINO "/dev/ttyACM0"

#define EXPLORE "e|"
#define RUN "r|"

int fd_ip=-1, fd_rfcomm=-1, fd_serial=-1;
int fd_ip_server, fd_rfcomm_server;
fd_set readfds;

int main(int argc, char *argv[]){
	char bf_ip[SIZE] = "", bf_rfcomm[SIZE] = "", bf_seial[SIZE] = "";
	char buffer[20] = "";

	int port_no = atoi(argv[1]);
	uint32_t svc_uuid_int[] = {0x1101, 0x1000, 0x80000080 ,0x5f9b34fb};

	FD_ZERO(&readfds);

	printf("Program start up!\n");

	// setup_serial(BAUD, DEVICE_ARDUINO);
	// setup_rfcomm(svc_uuid_int);
	setup_ip(port_no);

	fflush(stdin);
	FD_SET(STDIN, &readfds);
	fd_set readfds_temp;
	while(1){
		readfds_temp = readfds;
		
		select(FD_SETSIZE, &readfds_temp, NULL, NULL, NULL);

		// just for jumping out of loop through command
		if (FD_ISSET(STDIN, &readfds_temp)){
			scanf("%s", buffer);
			if(strcmp(buffer, "end")==0){
				break;
			}
		}

		// if (FD_ISSET(fd_rfcomm_server, &readfds_temp)){
		// 	accept_rfcomm();
		// }

		if (FD_ISSET(fd_ip_server, &readfds_temp)){
			accept_ip();
		}

		//if all three connections are established, start reading
		//ip or rfcomm writing should be before serial.
		// if(fd_rfcomm > 0 && fd_serial > 0 && fd_ip > 0){
			// if (FD_ISSET(fd_rfcomm, &readfds_temp)){
			// 	read_rfcomm(bf_rfcomm);
			// 	if(strcmp(bf_rfcomm, EXPLORE)==0)
			// 		write_serial(bf_rfcomm);
			// 	else if(strcmp(bf_rfcomm, RUN)==0)
			// 		write_ip(bf_rfcomm);
			// 	bzero(bf_rfcomm,sizeof(bf_rfcomm));
			// }
			// if (FD_ISSET(fd_serial, &readfds_temp)){
			// 	read_serial(bf_seial);
			// 	write_ip(bf_seial);
			// 	bzero(bf_seial,sizeof(bf_seial));
			// }
			if (FD_ISSET(fd_ip, &readfds_temp)){
				read_ip(bf_ip);
				write_ip(bf_ip);
				// write_serial(bf_ip);
				bzero(bf_ip,sizeof(bf_ip));
			}
		}
	// }

	close_ip();
	// close_rfcomm();
	// close_serial();

	printf("Programm terminating...\n");
}