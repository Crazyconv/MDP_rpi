#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

char device[] = "/dev/ttyACM";
//unsigned char newChar;

char buffer[256] = "";

int fd;
unsigned long baud = 115200;

int main(int argc, char *argv[]){
	printf("Raspberry Pi Startup!\n");
	fflush(stdout);
	char newChar;
	int index = 0;

	strcat(device,argv[1]);
	if((fd = serialOpen(device, baud)) < 0){
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		exit(1);
	}
	while(1){
		printf("send message to arduino: ");
		scanf("%s", &buffer);
		printf("%s\n", buffer);
		serialPuts (fd,buffer);
		if(strcmp(buffer, "X|")==0){
			bzero(buffer, 256);
			while(1){
				if(serialDataAvail(fd)){
					newChar = serialGetchar(fd);
					if(newChar == '|'){
						buffer[index] = '\0';
						printf("sensor reading: %s\n",buffer);
						index = 0;
						break;
					}else{
						buffer[index] = newChar;
						index ++;
					}
				}
			}
		}
		bzero(buffer, 256); 
	}
	//newChar = serialGetchar(fd);
	//printf("receive character to arduino: %c\n", newChar);
}
