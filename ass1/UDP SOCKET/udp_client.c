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
    socklen_t len;

    if (argc < 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Failed to create socket.\n");
        exit(1);
    }
    printf("UDP socket created.\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    len = sizeof(serv_addr);

    while (1) {
        printf("Enter message: ");
        fgets(buffer, 1024, stdin);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*) &serv_addr, len);

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Goodbye to server.\n");
            break;
        }

        bzero(buffer, 1024);
        n = recvfrom(sockfd, buffer, 1024, 0, NULL, NULL);
        if (n < 0) {
            printf("Failed to receive message.\n");
            exit(1);
        }

        printf("Server reply: %s", buffer);

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Server ended the chat.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}

