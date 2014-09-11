#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>

#include "client_ip.h"

// connect to server: rpi
// if successfully connected, return file descriptor of socket
// otherwise return -1
int setup_ip(int port_no, char *ip_address){
	struct sockaddr_in addr_server = { 0 };
    struct hostent *server;

    int fd_ip = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ip < 0) 
        perror("ERROR opening socket\n");
    else {
    	server = gethostbyname(ip_address);
    	if (server == NULL)
    		perror("No such host.\n");
    	else {
    		addr_server.sin_family = AF_INET;
    		bcopy((char *)server->h_addr, (char *)&addr_server.sin_addr.s_addr, server->h_length);
    		addr_server.sin_port = htons(port_no);
    		while (connect(fd_ip,(struct sockaddr *)&addr_server,sizeof(addr_server)) < 0) 
        		perror("ERROR connecting.\n");
    		printf("Connect to server %s\n", ip_address);

    		return fd_ip;
    	}
    }
    // fail to connect
    return -1;
}

// read from rpi to bf_read
// if disconnection occurs, return -1
// else return 0;
int read_ip(int fd_ip, char* bf_read){
	// signal of disconnection
	if(read(fd_ip,bf_read,SIZE) < 0){
		printf("Disconnected from Rpi.\n");
		close(fd_ip);
		return -1;
	}
	// may delete this
	else {
		printf("Receive message from PC: %s\n", bf_read);
		return 0;
	}
}

void write_ip(int fd_ip, char* bf_write){
	if(strlen(bf_write)>0){
		write(fd_ip, bf_write, strlen(bf_write));
		// may delete this
		printf("Write message from PC: %s\n", bf_write);
	}
}

void close_ip(int fd_ip){
	close(fd_ip);
}

