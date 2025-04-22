
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define SERV_TCP_PORT 23 /* server's port number */
#define MAX_SIZE 80

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg); // Don't leak memory!

    char buffer[80];
    for(;;) {
        int len = read(client_sock, buffer, sizeof(buffer) - 1);

        if (len <= 0)
            break;

        buffer[len] = '\0'; // Null-terminate first!

        if (strcmp(buffer, "close") == 0)
            break;

        printf("Client says: %s\n", buffer);
    }
    close(client_sock);
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;
    int port;
    char string[MAX_SIZE];
    int len;

    /* command line: server [port_number] */

    if(argc == 2)
        sscanf(argv[1], "%d", &port); /* read the port number if provided */
    else
        port = SERV_TCP_PORT;

    /*  open a TCP socket (an Internet stream socket)
        AF_INET is IPCV4, a widely used internet protocol
        SOCK_STREAM is a socket type.
    */

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
         perror("can't open stream socket");
         exit(1);
    }

    /* bind the local address, so that the client can send to server */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // serv_addr is a struct, which is kinda a class with different variables. We are setting certain variables with values
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // if bind is <0, there is an error.
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("can't bind local address");
        exit(1);
    }

    // listen to the socket
    listen(sockfd, 5); // 5 is the maximum number we expect the queue to grow.

    // do forever until breaks
    // In main server loop:
    for (;;) {
        clilen = sizeof(cli_addr);
        int *newsockfd = malloc(sizeof(int));
        *newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        printf("sockaddr = (%i)", *newsockfd);

        if (*newsockfd < 0) {
            perror("accept failed");
            free(newsockfd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, newsockfd);
        pthread_detach(tid); // Optional: makes the thread clean up after itself

        printf("\nCreated a new handle_client thread\n");
    }

}

//  Note: gcc tcp_server.c -lpthread -o server

/*

Get it working so that server and client can send multiple messages and terminate with a command word.

a database or something to store the messages

Allow the client to see the messages (might need to change the server AND client)

And ID system so we visually know which client is which (usernames)

*/




