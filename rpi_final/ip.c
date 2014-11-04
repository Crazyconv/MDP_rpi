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

// open socket and wait for ip connection
void setup_ip(int port_no){
	struct sockaddr_in addr_server = { 0 };
	int fd_temp;

	// create and bind socket to address
	fd_ip_server = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_ip_server < 0){
		perror("ERROR opening ip socket.\n");
	} else {
		addr_server.sin_family = AF_INET;
		addr_server.sin_addr.s_addr = INADDR_ANY;
		addr_server.sin_port = htons(port_no);
		if(bind(fd_ip_server, (struct sockaddr *) &addr_server, sizeof(addr_server)) < 0)
			perror("ERROR on binding.\n");
		else{
			FD_SET(fd_ip_server, &readfds);
			listen(fd_ip_server,5);
			printf("Listen to port %d for ip socket connection.\n", port_no);
		}
	}
	
}

// accept connection request and put fd_ip to fd set
void accept_ip(){
	struct sockaddr_in addr_client = { 0 };
	int fd_temp;

	// wait for client to connect
	int len_client = sizeof(addr_client);
	if((fd_temp = accept(fd_ip_server, (struct sockaddr *) &addr_client, &len_client)) >= 0){
		fd_ip = fd_temp;
		FD_SET(fd_ip, &readfds);
		printf("Accept ip socket connection from PC\n");
		//write_ip("Hello world\n");
	}
}

void read_ip(char* bf_ip){
	if(read(fd_ip,bf_ip,SIZE)>0){
		printf("Receive message from PC: %s\n", bf_ip);
		if(bf_ip[0] == 'P'){
			sp = 1;
			bzero(bf_ip,strlen(bf_ip));
		}		
	} else {
		printf("Disconnected from PC.\n");
		close(fd_ip);
		FD_CLR(fd_ip, &readfds);
	}
}


// write the content of buffer to ip socket
void write_ip(char* buffer){
	if(strlen(buffer)>0){
		strcat(buffer, "\n");
		write(fd_ip, buffer, strlen(buffer));
		printf("Write message to PC: %s\n", buffer);
	}
}

// close connection
void close_ip(){
	close(fd_ip);
	close(fd_ip_server);
	printf("Close ip socket connection.\n");
}
