#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#include "global.h"
#include "serial.h"

void setup_serial(int baud, char *device){
	fd_serial = serialOpen(device, baud);
	if(fd_serial < 0){
		perror("ERROR opening serial device.\n");
	} else {
		FD_SET(fd_serial, &readfds);
		printf("Serial port connection established.\n");
	}
}

void read_serial(char *bf_serial){
	int index = 0;
	char newChar;
	while(1){
		if(serialDataAvail(fd_serial)){
			newChar = serialGetchar(fd_serial);
			if(newChar == '|'){
				bf_serial[index] = '|';
				bf_serial[index+1] = '\0';
				printf("Receive message from serial:%s\n",bf_serial);
				break;
			} else {
				bf_serial[index] = newChar;
				index ++;
			}
		}
	}
}

void write_serial(char *buffer){
	//char buffer2[256] = "";
	if(strlen(buffer)>0){
		//strcat(buffer, "|");
		//if(buffer[strlen(buffer)-1]=='\n'){
		//	strncpy(buffer2, buffer, strlen(buffer)-1);
		//}else{
		//	strncpy(buffer2, buffer, strlen(buffer));
		//}
		serialPuts(fd_serial, buffer);
		printf("Write message to serial: %s\n", buffer);
	}
}

void close_serial(){
	serialClose(fd_serial);
	printf("Close serial port.\n");
}
