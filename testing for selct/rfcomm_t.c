#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

#include "rfcomm.h"
#include "global.h"

#define STDIN 0

int fd_rfcomm = -1;
int fd_rfcomm_server;
char bf_rfcomm[256] = "";
pthread_mutex_t mutex_fd_rfcomm;
pthread_mutex_t mutex_readfds;
fd_set readfds;

sdp_session_t *register_service(uint8_t rfcomm_port, uint32_t *svc_uuid_int){
	// service connection, just for humnan to read
	const char *service_name = "Bluetooth Insecure";
	const char *service_dsc = "For bluetooth connection";
	const char *service_prov = "Bluetooth";

	// local variables that will be used to store the different data elements of the service record
	uuid_t svc_uuid,
			svc_class_uuid,
			root_uuid, 
			l2cap_uuid, 
			rfcomm_uuid;
	sdp_list_t *svc_class_list = 0,
			*profile_list = 0,
			*root_list = 0,
			*l2cap_list = 0,
			*rfcomm_list = 0,
			*proto_list = 0,
			*access_proto_list = 0;

	sdp_data_t *channel = 0;
	sdp_profile_desc_t profile;

	sdp_record_t record = { 0 };
	sdp_session_t *session = 0;

	// PART ONE

	// set the general service ID
	sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
	sdp_set_service_id(&record, svc_uuid);

	// set the service class
	sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
	svc_class_list = sdp_list_append(0, &svc_class_uuid);
	sdp_set_service_classes(&record, svc_class_list);

	// set the Bluetooth profile information
	sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
	profile.version = 0x0100;
	profile_list = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, profile_list);

	// make the service record publicly browsable
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root_list = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root_list);

	// set l2cap information
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append(0, &l2cap_uuid);
	proto_list = sdp_list_append(0, l2cap_list);

	// register the RFCOMM channel for RFCOMM sockets
	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	channel = sdp_data_alloc(SDP_UINT8, &rfcomm_port);
	rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
	sdp_list_append(rfcomm_list, channel);
	sdp_list_append(proto_list, rfcomm_list);

	// set access protocal
	access_proto_list = sdp_list_append(0, proto_list);
	sdp_set_access_protos(&record, access_proto_list);

	// set the name, provider, and description
	sdp_set_info_attr(&record, service_name, service_prov, service_dsc);

	// PART TWO

	// connect to the local SDP server, register the service record and connect
	session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, 0);
	sdp_record_register(session, &record, 0);

	// cleanup
	sdp_data_free(channel);
	sdp_list_free(l2cap_list, 0);
	sdp_list_free(rfcomm_list, 0);
	sdp_list_free(root_list, 0);
	sdp_list_free(access_proto_list, 0);
	sdp_list_free(proto_list, 0);
	sdp_list_free(svc_class_list, 0);
	sdp_list_free(profile_list, 0);

	return session;
}

void *setup_rfcomm(void *arg){
	pthread_t thread_read;
	uint32_t svc_uuid_int[] = {0x1101, 0x1000, 0x80000080 ,0x5f9b34fb};

	struct sockaddr_rc addr_server = { 0 }, addr_client = { 0 };
	int port, fd_temp;
	socklen_t opt = sizeof(addr_client);
	char mac_client[256] = "";

	// allocate socket
	fd_rfcomm_server = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if(fd_rfcomm_server < 0){
		perror("ERROR opening socket.\n");
	}

	// bind socket to port 1 of the first available local bluetooth adapter
	addr_server.rc_family = AF_BLUETOOTH;
	addr_server.rc_bdaddr = *BDADDR_ANY;
	// dynamically assigned port number
	for(port = 1; port < 30; port++){
		addr_server.rc_channel = port;
		if(bind(fd_rfcomm_server, (struct sockaddr *)&addr_server, sizeof(addr_server)) == 0)
			break;
	}

	// register bluetooth service with SDP
	sdp_session_t* session = register_service((uint8_t)port, svc_uuid_int);

	listen(fd_rfcomm_server, 1);
	printf("Listening to rfcomm in channel %d\n", port);

	// ===============================================================================
	// fd_rfcomm = accept(fd_rfcomm_server, (struct sockaddr *)&addr_client, &opt);
	// ba2str(&addr_client.rc_bdaddr, mac_client);
	// printf("Accept connection from %s\n", mac_client);
	// ===============================================================================

	// once a connection request comes in, close the older one and accept the new one
	// this is for reconnection when disconnected from PC	
	while((fd_temp = accept(fd_rfcomm_server, (struct sockaddr *)&addr_client, &opt)) >= 0){
		// // if already connected, close socket
		// if(fd_rfcomm >= 0){
		// 	pthread_mutex_lock(&mutex_fd_rfcomm);
		// 	//close(fd_rfcomm);
		// 	fd_rfcomm = fd_temp;
		// 	pthread_mutex_unlock(&mutex_fd_rfcomm);
		// } else {
		// 	fd_rfcomm = fd_temp;
		// }

		// update file descriptor
		pthread_mutex_lock(&mutex_fd_rfcomm);
		fd_rfcomm = fd_temp;
		pthread_mutex_unlock(&mutex_fd_rfcomm);

		// add file descriptior to listen list
		pthread_mutex_lock(&mutex_readfds);
		FD_SET(fd_rfcomm, &readfds);
		pthread_mutex_unlock(&mutex_readfds);

		ba2str(&addr_client.rc_bdaddr, mac_client);
		printf("Accept connection from %s\n", mac_client);

		// create a new thread to read from the new port;
		//pthread_create(&thread_read, NULL, from_rfcomm, (void*)1);
	}

	printf("Socket closed. Stop transmitting.\n");
	pthread_exit(NULL);
}

void close_rfcomm(){
	close(fd_rfcomm);
	close(fd_rfcomm_server);
	printf("Close rfcomm socket.\n");
}

void multiplex(){
	char buffer[256]="";	
	fd_set readfds_temp;
	// struct timeval tval;
	// tval.tv_sec = 5;
	// tval.tv_usec = 0;

	while(1){
		pthread_mutex_lock(&mutex_readfds);
		readfds_temp = readfds;
		pthread_mutex_unlock(&mutex_readfds);
		
		select(FD_SETSIZE, &readfds_temp, NULL, NULL, NULL);
		if (FD_ISSET(STDIN, &readfds_temp)){
			scanf("%s",buffer);
			//printf("A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!A key was pressed lor!\n");
			printf("Received: %s\n",buffer);
		}
		if (FD_ISSET(fd_rfcomm, &readfds_temp)){
			if(read(fd_rfcomm,bf_rfcomm,sizeof(bf_rfcomm))>=0){
				printf("receive mesage from client: %s\n", bf_rfcomm);
				if(strcmp(bf_rfcomm,"end")==0){
					break;
				}
				pthread_mutex_lock(&mutex_fd_rfcomm);
				write(fd_rfcomm, bf_rfcomm, sizeof(bf_rfcomm));
				pthread_mutex_unlock(&mutex_fd_rfcomm);

				bzero(bf_rfcomm,sizeof(bf_rfcomm));
			} else {
				printf("Disconnected.\n");
				close(fd_rfcomm);
				pthread_mutex_lock(&mutex_readfds);
				FD_CLR(fd_rfcomm, &readfds);
				pthread_mutex_unlock(&mutex_readfds);
			}
		}
		printf("timeout\n");
	}
}

int main(){
	printf("Program start up!\n");

	// set up file description set
	fflush(stdin);
	FD_ZERO(&readfds);
	FD_SET(STDIN, &readfds);

	// create threads for each dataflow
	pthread_attr_t attr;
	pthread_t threads;

	pthread_mutex_init(&mutex_fd_rfcomm,NULL);
	pthread_mutex_init(&mutex_readfds,NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create(&threads,&attr, setup_rfcomm, (void*)1);

	while(fd_rfcomm<0);
	multiplex();

	close_rfcomm();
	pthread_join(threads,NULL);

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex_fd_rfcomm);
	pthread_mutex_destroy(&mutex_readfds);

	printf("Program terminating...\n");

	pthread_exit(NULL);
}

