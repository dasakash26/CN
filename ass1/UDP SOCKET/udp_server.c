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
    struct sockaddr_in serv_addr, client_addr;
    char buffer[1024];
    socklen_t len;

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Failed to create socket.\n");
        exit(1);
    }
    printf("UDP socket created.\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Failed to bind socket.\n");
        exit(1);
    }
    printf("UDP server is listening on port %d...\n", portno);

    len = sizeof(client_addr);

    while (1) {
        bzero(buffer, 1024);
        n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*) &client_addr, &len);
        if (n < 0) {
            printf("Failed to receive message.\n");
            exit(1);
        }

        printf("Client [%s:%d] says: %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Goodbye from client.\n");
            break;
        }

        printf("Enter reply: ");
        fgets(buffer, 1024, stdin);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*) &client_addr, len);

        if (strncmp("bye", buffer, 3) == 0) {
            printf("Goodbye from server.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}

