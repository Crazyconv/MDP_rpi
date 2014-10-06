#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

char device[] = "/dev/ttyACM0";
unsigned char newChar;

int fd;
unsigned long baud = 115200;

int main(){
	printf("Raspberry Pi Startup!\n");
	fflush(stdout);

	if((fd = serialOpen(device, baud)) < 0){
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		exit(1);
	}

	printf("send character to arduino: ");
	scanf("%c", &newChar);
	serialPutchar (fd, newChar); 

	newChar = serialGetchar(fd);
	printf("receive character to arduino: %c\n", newChar);
}
