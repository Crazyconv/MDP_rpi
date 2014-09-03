/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    char buffer[256] = "";
    int n;
    int fd_ip;
     // ip socket connection with PC
    struct sockaddr_in addr_server = { 0 }, addr_client = { 0 };

    // create and bind socket to address
    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_socket < 0){
        errors("ERROR opening ip socket.\n");
    }
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = INADDR_ANY;
    addr_server.sin_port = htons(PORT_NO);
    if(bind(fd_socket, (struct sockaddr *) &addr_server, sizeof(addr_server)) < 0)
        errors("ERROR on binding.\n");
    printf("Rpi listening to port %d\n", PORT_NO);
    listen(fd_socket,1);

    // wait for client to connect
    int len_client = sizeof(addr_client);
    fd_ip = accept(fd_socket, (struct sockaddr *) &addr_client, &len_client);
    if(fd_ip < 0)
        errors("ERROR on ip accept.\n");
    printf("IP socket connection established.\n");

     bzero(buffer,256);
     n = read(fd_ip,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(fd_ip,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     return 0; 
}
