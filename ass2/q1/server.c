#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080
#define MAX_BUF 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX_BUF] = {0};
    char ack[] = "ACK";

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket creation failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    while (1) {
        // Receive data from client
        ssize_t len = recvfrom(server_fd, buffer, MAX_BUF, MSG_WAITALL, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        buffer[len] = '\0';  // Null terminate the received string

        printf("Received: %s\n", buffer);

        // Send acknowledgment
        sendto(server_fd, ack, strlen(ack), 0, (struct sockaddr *)&address, addrlen);
        printf("Sent ACK\n");
    }

    return 0;
}
