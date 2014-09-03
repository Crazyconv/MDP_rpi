#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>

#define SIZE 4

char buf_read[SIZE] = "";
char buf_write[SIZE] = "";
pthread_t threads[2];
int sockfd;

void errors(char *msg){
    perror(msg);
    exit(1);
}

void *readf(void *arg){
    while(1){
        bzero(buf_read,SIZE);
        if(read(sockfd,buf_read,SIZE) < 0){
            perror("Error reading from socket. Connection closed.\n");
            break;
        }
        printf("Receiving message from Arduino: %s\n",buf_read);
    }
    pthread_exit(NULL);
}

void *writet(void *arg){
    while(1){
        printf("Send message to Arduino: ");
        fflush(stdin);
        bzero(buf_write,SIZE);
        scanf("%s",buf_write);
        if(write(sockfd, buf_write, sizeof(buf_write))<0){
            perror("Error writing to socket. Connection closed.\n");
            break;
        }
        printf("Sent successfully!\n");
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int portno = atoi(argv[1]);
    //portno = 5002;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        errors("ERROR opening socket");
    //server = gethostbyname("192.168.22.1");
    server = gethostbyname("localhost");
    if (server == NULL)
        errors("No such host.\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        errors("ERROR connecting");
    printf("socket: %d\n",sockfd);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0],&attr, readf, (void*)sockfd);
    pthread_create(&threads[1],&attr, writet, (void*)sockfd);

    pthread_join(threads[0],NULL);
    pthread_join(threads[1],NULL);
    pthread_attr_destroy(&attr);

    if(close(sockfd)<0){
        perror("Connection already closed.\n");
    }

    printf("Program terminating...\n");

    pthread_exit(NULL);
    return 0;
}
