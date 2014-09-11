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

// open socket and wait for rfcomm connection 
void setup_rfcomm(uint32_t *svc_uuid_int){
	struct sockaddr_rc addr_server = { 0 };
	int port;

	// allocate socket
	fd_rfcomm_server = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if(fd_rfcomm_server < 0){
		perror("ERROR opening rfcomm socket.\n");
	} else {
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

		FD_SET(fd_rfcomm_server, &readfds);
		listen(fd_rfcomm_server, 1);
		printf("Listening to channel %d for rfcomm connection.\n", port);
	}	
}

// accept connection request and put fd_rfcomm to fd set
void accept_rfcomm(){
	int fd_temp;
	struct sockaddr_rc addr_client = { 0 };
	socklen_t opt = sizeof(addr_client);
	char mac_client[256] = "";

	if((fd_temp = accept(fd_rfcomm_server, (struct sockaddr *)&addr_client, &opt)) >= 0){
		fd_rfcomm = fd_temp;
		FD_SET(fd_rfcomm, &readfds);
		ba2str(&addr_client.rc_bdaddr, mac_client);
		printf("Accept rfcomm connection from %s\n", mac_client);
	}
}

// read from fd_rfcomm
// if disconnected, close socket and delete fd_rfcomm from fd set
int read_rfcomm(char* bf_rfcomm){
	if(read(fd_rfcomm,bf_rfcomm,SIZE)>=0){
		printf("receive mesage from bluetooth: %s\n", bf_rfcomm);
		if(strcmp(bf_rfcomm,"end")==0){
			return STOP;
		}
	} else {
		printf("Disconnected from bluetooth.\n");
		close(fd_rfcomm);
		FD_CLR(fd_rfcomm, &readfds);
	}
	return CONTINUE;
}

// write the content of buffer to rfcomm socket
void write_rfcomm(char* buffer){
	if(strlen(buffer)>0){
		write(fd_rfcomm, buffer, strlen(buffer));
		printf("Write message to bluetooth: %s\n", buffer);;
	}
}

// close socket
void close_rfcomm(){
	close(fd_rfcomm);
	close(fd_rfcomm_server);
	printf("Close rfcomm connection.\n");
}