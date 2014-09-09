#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#include "global.h"
#include "ip.h"

#define PORT_NO 5000

int fd_ip = -1;
int fd_ip_server;
pthread_mutex_t mutex_fd_ip;
char bf_ip[256] = "";

void *setup_ip(void *arg){
	// ip socket connection with PC
	struct sockaddr_in addr_server = { 0 }, addr_client = { 0 };
	pthread_t thread_read;
	int portno, fd_temp;
	printf("Port No: ");
	scanf("%d",&portno);

	// create and bind socket to address
	fd_ip_server = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_ip_server < 0){
		perror("ERROR opening ip socket.\n");
	}
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(portno);
	if(bind(fd_ip_server, (struct sockaddr *) &addr_server, sizeof(addr_server)) < 0)
		perror("ERROR on binding.\n");
	printf("Rpi listening to port %d\n", portno);
	listen(fd_ip_server,5);

	// wait for client to connect
	int len_client = sizeof(addr_client);
	while((fd_temp = accept(fd_ip_server, (struct sockaddr *) &addr_client, &len_client)) >= 0){
		if(fd_ip >= 0){
			pthread_mutex_lock(&mutex_fd_ip);
			close(fd_ip);
			fd_ip = fd_temp;
			pthread_mutex_unlock(&mutex_fd_ip);
		} else {
			fd_ip = fd_temp;
		}
		printf("Accept connection from PC.\n");
		pthread_create(&thread_read, NULL, from_ip, (void*)1);
	}

	printf("Socket closed. Stop transmitting.\n");
	pthread_exit(NULL);
}

void *from_ip(void *arg){
	//printf("From PC to Arduino.\n");
	while(1){
		bzero(bf_ip,sizeof(bf_ip));
		if(read(fd_ip,bf_ip,sizeof(bf_ip)) < 0)
			break;
		//pthread_mutex_lock(&mutex_serial);
		//serialPuts(fd_serial, bf_ip);
		//pthread_mutex_unlock(&mutex_serial);
		//printf("From PC to arduino: %s\n", bf_ip);
		printf("Receive: %s\n",bf_ip);
	}

	printf("Disconnection occurs. Thread terminating...\n");
	pthread_exit(NULL);
}

void close_ip(){
	close(fd_ip);
	close(fd_ip_server);
	printf("Close from socket.\n");
}

int main(){
	printf("Program start up!\n");

	// create threads for each dataflow
	pthread_attr_t attr;
	pthread_t threads;

	pthread_mutex_init(&mutex_fd_ip,NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&threads,&attr, setup_ip, (void*)1);

	pthread_join(threads,NULL);

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex_fd_ip);

	printf("Program terminating...\n");

	pthread_exit(NULL);
}