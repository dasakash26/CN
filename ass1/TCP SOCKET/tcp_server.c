#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {

    int sockfd, newsockfd, portno, n;
    struct sockaddr_in serv_addr, client_addr;
    char buffer[1024];
    socklen_t len;

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to open socket.\n");
        exit(1);
    }
    printf("Socket created successfully.\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Failed to bind socket.\n");
        exit(1);
    }
    printf("Socket bound to port %d.\n", portno);

    listen(sockfd, 5);
    printf("Listening for connections...\n");

    len = sizeof(client_addr);
    newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &len);
    if (newsockfd < 0) {
        printf("Failed to accept connection.\n");
        exit(1);
    }
    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    while (1) {
        bzero(buffer, 1024);
        printf("Waiting for client message...\n");

        n = read(newsockfd, buffer, 1024);
        if (n < 0) {
            printf("Failed to read from socket.\n");
            exit(1);
        }

        printf("Client message: %s\n", buffer);

        printf("Enter reply: ");
        fgets(buffer, 1024, stdin);

        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) {
            printf("Failed to write to socket.\n");
            exit(1);
        }

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Goodbye!\n");
            break;
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}

