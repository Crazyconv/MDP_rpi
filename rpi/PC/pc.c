/* This is a sample program for PC to read and write from Rpi
 * Use ip socket programming
 * Assume PC only perform writing after reading something
 * Just insert your code in the indicated place
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "client_ip.h"

#define SERVER "192.168.22.1"
#define PORT_NO 5000
#define SIZE 256

int main(){
	char bf_read[SIZE] = "", bf_write[SIZE] = "";
	int fd_ip;
	printf("Program start up!\n");

	//fd_ip = setup_ip(PORT_NO, "192.168.22.1");
	fd_ip = setup_ip(PORT_NO, "localhost");
	if(fd_ip == -1){
		// fail to connect
		exit(1);
	}

	// just use in sample
	int i = 0; 
	while (1){
		if(read_ip(fd_ip, bf_read) == -1){
			break;
		}

		// ============== INSERT YOUR CODE HERE ==============
		// ============== USE CONTENT IN BF_READ =============
		// ===== WRITE WHAT YOU WANT TO SENT TO BF_WRITE ===== 
		// ====================== sample =====================
		sprintf(bf_write, "%d", i);
		i++;
		// ===================================================
		
		write_ip(fd_ip, bf_write);
		bzero(bf_read, SIZE);
		bzero(bf_write, SIZE);
	}
	
	void close_ip(int fd_ip);
}