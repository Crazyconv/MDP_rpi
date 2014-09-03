#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

char device[] = "/dev/ttyACM0";
char message[256] = "";
char newChar;

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

	serialPuts(fd,"Hello Arduino");
	// while(1){
	// 	serialPuts(fd,"Hello Arduino");
	// 	printf("%d iteration\n",i);
	// 	while(serialDataAvail(fd)){
	// 		newChar = serialGetchar(fd);
	// 		if(newChar == '\0'){
	// 			printf("%d iteration: receiving %s\n",i,message);
	// 			bzero(message,256);
	// 			break;
	// 		} else {
	// 			message[strlen(message)] = newChar;
	// 		}
	// 	}
	// 	i++;
	// 	sleep(1);
	// }
	while(1){
		printf("%d iteration\n",i);
		while(serialDataAvail(fd)){
			newChar = serialGetchar(fd);
			if(newChar == '\n'){
				printf("%d iteration: receiving %s\n",i,message);
				bzero(message,256);
				break;
			} else {
				message[strlen(message)] = newChar;
			}
		}
		i++;
		sleep(1);
	}
	return 0;
}