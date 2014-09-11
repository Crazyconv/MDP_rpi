#ifndef Rfcomm_h
#define Rfcomm_h

#include <stdint.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

sdp_session_t *register_service(uint8_t rfcomm_port, uint32_t *svc_uuid_int);;
void setup_rfcomm();
void multiplex();
void close_rfcomm();

#endif