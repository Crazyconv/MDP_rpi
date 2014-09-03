/* This is a multi-thread program for passing data between PC, arduino and android device  
 *
 * Need to develop protocal: size of the message passed
 * 
 * Put everything in one file temporary
 * Need to address the problem of run on start
 * Need to address the problem of sequences of establishing connections
 * Need to address the problem of whether rpi serves as the client or server for bluetooth
 * For now, RPI serves as the client
 * For RPI to serve as the server, android need to develope server code first
 * May need to remove the error messge
 * May need to close connection before terminating
 *
 * Need to hardcode portNo
 * Need to hardcode channel
 * 
 * Data flow: Arduino - PC+Arduino, android - Arduino, PC - Arduino
 * Data read and write only after all three connections are established
 * Communication stops when receiving "terminate" from android
 *
 * Compile using: gcc rpi.c -lpthread -lwiringPi -lbluetooth
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

#define SIZE 4
#define PORT_NO 5000
#define DEVICE_BLUETOOTH "AC:F7:F3:53:31:BE"
#define CHANNEL_BLUETOOTH 2
#define BAUD 9600
#define DEVICE_ARDUINO "/dev/ttyACM0"

// File descriptors for ip socket, rfcomm socket and serial port
int fd_ip, fd_rfcomm, fd_serial;

// Buffers for data read from PC(ip), android(rfcomm) and arduino(serial)
char bf_ip[SIZE] = "";
char bf_rfcomm[SIZE] = "";
char bf_serial[SIZE] = "";

// Mutex for each buffer
pthread_mutex_t mutex_ip, mutex_rfcomm, mutex_serial;
pthread_t threads[3];

// Display error message and exit on error
void errors(char *msg){
	perror(msg);
	exit(1);
}

void setup_ip(){
	// ip socket connection with PC
	struct sockaddr_in addr_server = { 0 }, addr_client = { 0 };
	int portno;
	printf("Port No: ");
	scanf("%d",&portno);

	// create and bind socket to address
	int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_socket < 0){
		errors("ERROR opening ip socket.\n");
	}
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(portno);
	if(bind(fd_socket, (struct sockaddr *) &addr_server, sizeof(addr_server)) < 0)
		errors("ERROR on binding.\n");
	printf("Rpi listening to port %d\n", PORT_NO);
	listen(fd_socket,1);

	// wait for client to connect
	int len_client = sizeof(addr_client);
	fd_ip = accept(fd_socket, (struct sockaddr *) &addr_client, &len_client);
	if(fd_ip < 0)
		errors("ERROR on ip accept.\n");
	printf("IP socket connection established.\n");
}

void setup_rfcomm(){
	// rfcomm socket connection with android device
	printf("Please enter the bluetooth channel: ");
	int channel;
	scanf("%d",&channel);
	struct sockaddr_rc addr_rpi = { 0 };

	// create and bind socket to address
	fd_rfcomm = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if(fd_rfcomm < 0){
		errors("ERROR opening rfcomm socket.\n");
	}
	addr_rpi.rc_family = AF_BLUETOOTH;
	addr_rpi.rc_channel = (uint8_t) channel;
	str2ba( DEVICE_BLUETOOTH, &addr_rpi.rc_bdaddr );
	while(connect(fd_rfcomm, (struct sockaddr *)&addr_rpi, sizeof(addr_rpi)) < 0);
	printf("RFCOMM socket connection established.\n");
}

void setup_serial(){
	// serial port connection with arduino
	fd_serial = serialOpen(DEVICE_ARDUINO, BAUD);
	if(fd_serial < 0){
		errors("ERROR opening serial device.\n");
	}
	printf("Serial port connection established.\n");
}

// Thread for reading from PC(ip) and writing to Arduino(serial)
void *from_ip(void *arg){
	printf("From PC to Arduino.\n");
	while(1){
		bzero(bf_ip,SIZE);
		if(read(fd_ip,bf_ip,SIZE) < 0)
			perror("ERROR reading from PC\n");
		pthread_mutex_lock(&mutex_serial);
		serialPuts(fd_serial, bf_ip);
		pthread_mutex_unlock(&mutex_serial);
		printf("From PC to arduino: %s\n", bf_ip);
	}
	pthread_exit(NULL);
}

// Thread for reading from Android(rfomm) and writing to Arduino(serial)
void *from_rfcomm(void *arg){
	printf("From Android to Arduino.\n");
	while(1){
		bzero(bf_rfcomm,SIZE);
		if(read(fd_rfcomm,bf_rfcomm,SIZE) < 0)
			perror("ERROR reading from android\n");
		//if "terminate" is received, stop communication of 3 devices
		if(strcmp(bf_rfcomm,"terminate")==0){
			pthread_cancel(threads[0]);
			pthread_cancel(threads[2]);
			break;
		}
		pthread_mutex_lock(&mutex_serial);
		serialPuts(fd_serial, bf_rfcomm);
		pthread_mutex_unlock(&mutex_serial);
		printf("From android to arduino: %s\n", bf_rfcomm);
	}
	pthread_exit(NULL);
}

// Thread for reading from Arduino(serial) and writing to PC(ip)
void *from_serial(void *arg){
	printf("From Arduino to PC.\n");
	char newChar;
	int index = 0;
	while(1){
		if(index == 4){
			pthread_mutex_lock(&mutex_ip);
			write(fd_ip, bf_serial, sizeof(bf_serial));
			pthread_mutex_unlock(&mutex_ip);
			printf("From Arduino to PC: %s\n", bf_serial);
			pthread_mutex_lock(&mutex_rfcomm);
			write(fd_rfcomm, bf_serial, sizeof(bf_serial));
			pthread_mutex_unlock(&mutex_rfcomm);
			printf("From Arduino to Android: %s\n", bf_serial);
			bzero(bf_serial,SIZE);
			index = 0;
		}
		if(serialDataAvail(fd_serial)){
			newChar = serialGetchar(fd_serial);
			bf_serial[index] = newChar;
			index ++;
		}
	}
	pthread_exit(NULL);
}

int main(){
	printf("Rpi startup!\n");

	// set up
	setup_ip();
	setup_rfcomm();
	setup_serial();

	// create threads for each dataflow
	pthread_attr_t attr;

	pthread_mutex_init(&mutex_ip,NULL);
	pthread_mutex_init(&mutex_rfcomm,NULL);
	pthread_mutex_init(&mutex_serial,NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&threads[0],&attr, from_ip, (void*)1);
	pthread_create(&threads[1],&attr, from_rfcomm, (void*)2);
	pthread_create(&threads[2],&attr, from_serial, (void*)3);

	pthread_join(threads[0],NULL);
	pthread_join(threads[1],NULL);
	pthread_join(threads[2],NULL);

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex_ip);
	pthread_mutex_destroy(&mutex_rfcomm);
	pthread_mutex_destroy(&mutex_serial);

	//close connection
	close(fd_rfcomm);
	printf("Close rfcomm socket.\n");
	serialClose(fd_serial);
	printf("Close serial port.\n");
	close(fd_ip);
	printf("Close ip socket.\n");

	printf("Program terminating...\n");

	pthread_exit(NULL);
}