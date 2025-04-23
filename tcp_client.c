#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


// A client is very similar to a server except that it connects to the

#define SERV_TCP_PORT 23 /* server's port number */
#define MAX_SIZE 80




void* receive_messages(void* sockfd_ptr) {
    int sockfd = *(int*)sockfd_ptr;
    char buffer[1024];
    int len;

    while ((len = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[len] = '\0';
        printf("\n[Server]: %s\n", buffer);
        printf("Enter a message: "); // re-prompt user
        fflush(stdout);
    }

    printf("Disconnected from server.\n");
    exit(0);
    return NULL;
}



int main(int argc, char *argv[])
{
    printf("Initalizing tcp_client.\n");

    int sockfd;
    struct sockaddr_in serv_addr;
    char *serv_host = "localhost";
    struct hostent *host_ptr;
    int port;
    int buff_size = 0;


    /* command line: client [host [port]] */

    if(argc >= 2)
    {
        serv_host = argv[1]; /* from the parameters, reads the host if provided */
        printf("Selecting server host ip address: %s\n", serv_host);

        if(argc == 3)
        {
           sscanf(argv[2], "%d", &port); /* read the port number if provided */
        }

        printf("Selecting the port number: %i\n", port);
    }
    else
    {
        port = SERV_TCP_PORT;
        printf("Selected the default port number: %i\n", SERV_TCP_PORT);

    }



    /* not opening a tcp socket, but getting the address of the host*/
    if((host_ptr = gethostbyname(serv_host)) == NULL)
    {
        perror("gethostbyname error");
        exit(1);
    }


    //host_ptr is a struct, if the h_addrtype taken from host_ptr is not the right protocol, error.
    if(host_ptr->h_addrtype !=  AF_INET) {
        perror("unknown address type");
        exit(1);
    }

    // setting a few variables that are in the serv_addr struct to relevant values.
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
    serv_addr.sin_port = htons(port);


    /* Open a TCP Socket */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("can't open stream socket");
        exit(1);
    }
    printf("Connecting...\n");

    /* Connect to the Server */
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("can't connect to server");
        exit(1);
    }
    printf("Connected!\n");


    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &sockfd);


    // variables needed for message sending
    char message[100];
    char username[50];
    char final_message[300];

    // Ask for username
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin); //receiving keyboard input

    // WRITING A MESSAGE TO THE SERVER

    printf("Enter a message: ");
    for(;;)
    {

        fflush(stdout);
        fgets(message, sizeof(message), stdin); //receiving keyboard input

        //remove the newline if present
        username[strcspn(username, "\n")] = 0;
        message[strcspn(message, "\n")] = 0;

        //If the message is "close" we stop the loop
        if(strcmp(message, "close") == 0)
        {
            snprintf(final_message, sizeof(final_message), "<%s %s>", username, "has disconnected.");
            write(sockfd, final_message, strlen(final_message)); // send a disconnect message
            write(sockfd, "close", strlen("close")); // send the "close" string, which will signal the server to remove the socket.
            break;
        }


        //formats the final message with the username and message
        snprintf(final_message, sizeof(final_message), "[%s] %s", username, message);

        write(sockfd, final_message, strlen(final_message));


    }

    close(sockfd);

}

/*
TO RUN THIS CODE:
	gcc client.c -o client
	./client localhost 7777
	OR
	./client 127.0.0.1 7777

*/
