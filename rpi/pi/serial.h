#ifndef serial_h
#define serial_h

void setup_serial(int baud, char *device);
void read_serial(char *bf_serial);
void write_serial(char *buffer);
void close_serial();

#endif
