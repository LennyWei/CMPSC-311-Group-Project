
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define SERV_TCP_PORT 23 /* server's port number */
#define MAX_SIZE 80
#define MAX_CLIENTS 100


/*Utilizing MUTEX for the shared resource client_sockets[] */
int client_sockets[MAX_CLIENTS]; // Array to hold active sockets
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // For thread-safe access to the client_socket's list

//
int sockfd, newsockfd, clilen;
struct sockaddr_in cli_addr, serv_addr;
int port;
char string[MAX_SIZE];
int len;


// FUNCTION: CLOSES THE SOCKET WHEN PRESSING CTRL + C
void handle_sigint(int sig) {
    printf("\nCaught SIGINT (Ctrl+C), shutting down server...\n");

    if (sockfd >= 0) {
        close(sockfd);
        printf("Socket closed.\n");
    }

    exit(0); // Terminate the program
}


//FUNCTION: ADDS CLIENT TO PREDEFINED LIST IN ORDER FOR MESSAGES TO BE BROADCAST TO ALL CLIENTS
void add_client(int sockfd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {

        // if that list value is nothing, then give it the new sockfd
        if (client_sockets[i] == 0) {
            client_sockets[i] = sockfd;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


//FUNCTION: REMOVES CLIENT ONCE DISCONNECTED FROM THE SERVER
void remove_client(int sockfd) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; ++i) {

        // if that list value is the correct sockfd, then remove it.
        if (client_sockets[i] == sockfd) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


//FUNCTION: BROADCASTS MESSAGES TO ALL CLIENTS
void broadcast(const char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        int sock = client_sockets[i];
        if (sock != 0) {
            if (write(sock, message, strlen(message)) < 0) {
                perror("write to client failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

//HANDLER FOR HANDLING EACH CLIENT
void *handle_client(void *arg)
{
    int client_sock = *(int *)arg;
    free(arg); //frees up the memory that was malloc-ed

    char buffer[80];

    //will loop until the client sends the "close" command (just a string)
    for(;;)
    {
        int len = read(client_sock, buffer, sizeof(buffer) - 1);

        buffer[len] = '\0'; //null-terminates first so that the final character of the string is valid (all proper strings have a NULL at the end)

        //if the "close" command was sent, break the loop!
        if(strcmp(buffer, "close") == 0)
            break;

        printf("%s\n", buffer);

        //check the method for explanation
        broadcast(buffer, client_sock);
    }

    close(client_sock);
    remove_client(client_sock); //check the method for explanation

    return NULL;
}

int main(int argc, char *argv[])
{
    // Register the signal
    signal(SIGINT, handle_sigint);


    printf("Server Initalizing.\n");

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

    // listen to the socket (
    listen(sockfd, 5); // 5 is the maximum number we expect the queue to grow.
    printf("Listening for Connections...\n");

    // do forever until breaks
    // In main server loop:
    for (;;) {
        clilen = sizeof(cli_addr);

        // malloc is to make sure that the correct socketfd is given to the relevant thread.
        int *newsockfd = malloc(sizeof(int));
        *newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        printf("sockaddr = (%i)", *newsockfd);

        if (*newsockfd < 0) {
            perror("accept failed");
            free(newsockfd);
            continue;
        }

        printf("Connected with a client!\n");

        add_client(*newsockfd); //check the method for explanation

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, newsockfd);
        pthread_detach(tid); // Optional: makes the thread clean up after itself

        printf("Created a new handle_client thread\n\n");
    }

}

//  Note: gcc tcp_server.c -lpthread -o server

/*

Get it working so that server and client can send multiple messages and terminate with a command word.

a database or something to store the messages

Allow the client to see the messages (might need to change the server AND client)

And ID system so we visually know which client is which (usernames)

*/




