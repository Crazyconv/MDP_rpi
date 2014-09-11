#ifndef client_ip_h
#define client_ip_h

#define SIZE 256

int setup_ip(int port_no, char *ip_address);
int read_ip(int fd_ip, char* bf_read);
void write_ip(int fd_ip, char* bf_write);
void close_ip(int fd_ip);

#endif