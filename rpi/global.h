#ifndef global_h
#define global_h

#include <pthread.h>

// File descriptors for ip socket, rfcomm socket and serial port
extern int fd_ip, fd_rfcomm, fd_serial;
// File descriptors for server ip socket, server rfcomm socket;
extern int fd_ip_server, fd_rfcomm_server;

// Buffers for data read from PC(ip), android(rfcomm) and arduino(serial)
extern char bf_ip[256], bf_rfcomm[256], bf_serial[256];

// Mutex for each buffer
extern pthread_mutex_t mutex_ip, mutex_rfcomm, mutex_serial;

// Mutex for ip socket file description and rfcomm socket file description
extern pthread_mutex_t mutex_fd_ip, mutex_fd_rfcomm;

#endif