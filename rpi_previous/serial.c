#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include <wiringPi.h>
#include <wiringSerial.h>

#include "global.h"
#include "serial.h"

#define BAUD 9600
#define DEVICE_ARDUINO "/dev/ttyACM0"

int fd_serial;
char bf_serial[256];

void* setup_serial(void *arg){
	fd_serial = serialOpen(DEVICE_ARDUINO, BAUD);
	if(fd_serial < 0){
		errors("ERROR opening serial device.\n");
	}
	printf("Serial port connection established.\n");
}

void* from_serial(void *arg){
	char newChar;
	int index = 0;
	while(1){
		// if(index == 4){
		// 	pthread_mutex_lock(&mutex_ip);
		// 	write(fd_ip, bf_serial, sizeof(bf_serial));
		// 	pthread_mutex_unlock(&mutex_ip);
		// 	printf("From Arduino to PC: %s\n", bf_serial);
		// 	pthread_mutex_lock(&mutex_rfcomm);
		// 	write(fd_rfcomm, bf_serial, sizeof(bf_serial));
		// 	pthread_mutex_unlock(&mutex_rfcomm);
		// 	printf("From Arduino to Android: %s\n", bf_serial);
		// 	bzero(bf_serial,SIZE);
		// 	index = 0;
		// }
		if(serialDataAvail(fd_serial)){
			newChar = serialGetchar(fd_serial);
			if(newChar == '\\'){
				bf_serial[index] = '\0';
				index = 0;
				printf("receive: %s\n",bf_serial);
			}
			bf_serial[index] = newChar;
			index ++;
		}
	}
	pthread_exit(NULL);
}

void close_serial(){
	serialClose(fd_serial);
	printf("Close serial port.\n")
}