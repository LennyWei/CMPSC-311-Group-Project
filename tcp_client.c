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


// A client is very similar to a server except that it connects to the

#define SERV_TCP_PORT 23 /* server's port number */
#define MAX_SIZE 80

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
        serv_host = argv[1]; /* from the parameters, reads the host if provided */
        printf("Selecting server host ip address: %s\n", serv_host);

        if(argc == 3)
        {
           sscanf(argv[2], "%d", &port); /* read the port number if provided */
        }

        printf("Selecting the port number: %i\n", port);

    else
        port = SERV_TCP_PORT;
        printf("Selected the default port number: %i\n", SERV_TCP_PORT);


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

    /* Connect to the Server */
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("can't connect to server");
        exit(1);
    }
    printf("Connected!\n");





//WRITING A MESSAGE TO THE SERVER

    char message[50];

    for(;;)
    {
        printf("Enter a message: ");
        fgets(message, sizeof(message), stdin); //receiving input

        //removes the trailing newline so that it can be compared to the "terminate" command
        message[strcspn(message, "\n")] = 0;

        write(sockfd, message, strlen(message));

        if(strcmp(message, "close") == 0)
            break;
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
