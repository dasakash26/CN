#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    if (argc < 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to open socket.\n");
        exit(1);
    }
    printf("Socket created successfully.\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection to server failed.\n");
        exit(1);
    }
    printf("Connected to server at %s:%d\n", argv[1], portno);

    while (1) {
        printf("Enter message: ");
        fgets(buffer, 1024, stdin);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            printf("Failed to write to socket.\n");
            exit(1);
        }

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Disconnected from server.\n");
            break;
        }

        bzero(buffer, 1024);
        n = read(sockfd, buffer, 1024);
        if (n < 0) {
            printf("Failed to read from socket.\n");
            exit(1);
        }

        printf("Server reply: %s\n", buffer);

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Server said goodbye.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}

