#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

char device[] = "/dev/ttyACM0";

int fd;
unsigned long baud = 9600;
int i = 1;

int main(){
	printf("Raspberry Pi Startup!\n");
	fflush(stdout);

	if((fd = serialOpen(device, baud)) < 0){
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		exit(1);
	}

	while(1){
		serialPutchar(fd,'A');
		printf("%d iteration: sending A\n",i);
		if(serialDataAvail(fd)){
			printf("%d iteration: receiving %c\n",i,serialGetchar(fd));
			fflush(stdout);
		}
		i++;
		sleep(1);
	}
	
	return 0;
}