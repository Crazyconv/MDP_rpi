#ifndef global_h
#define global_h

#define SIZE 256

#include <sys/types.h>
#include <unistd.h>

// File descriptors for ip socket, rfcomm socket and serial port
extern int fd_ip, fd_rfcomm, fd_serial;
// File descriptors for server ip socket, server rfcomm socket;
extern int fd_ip_server, fd_rfcomm_server;

extern fd_set readfds;

// ===================
// Oct 6
extern int step;
extern int sp;
extern int no_sp;
extern int gts;
// ===================

#endif
