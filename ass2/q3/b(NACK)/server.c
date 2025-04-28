#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080
#define MAX_BUF 1024

struct frame {
    int seq_num;
    char data[MAX_BUF];
};

void receiver_nack(int sock) {
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    struct frame received_frame;
    char response[MAX_BUF];
    int expected_seq_num = 0;

    while (1) {
        // Receive frame from the sender
        ssize_t len = recvfrom(sock, &received_frame, sizeof(received_frame), MSG_WAITALL, (struct sockaddr *)&address, &addr_len);
        if (len > 0) {
            // Check if the received frame has the expected sequence number
            if (received_frame.seq_num == expected_seq_num) {
                printf("Received frame %d: %s\n", received_frame.seq_num, received_frame.data);

                // Create an ACK message
                snprintf(response, MAX_BUF, "ACK for %d", received_frame.seq_num);
                sendto(sock, response, strlen(response), 0, (struct sockaddr *)&address, addr_len);
                printf("Sent %s\n", response);

                // Move to the next expected frame
                expected_seq_num++;
            } else {
                // Frame out of order or missing, send NACK for the expected frame
                printf("Frame %d missing, sending NACK\n", expected_seq_num);
                snprintf(response, MAX_BUF, "NACK for %d", expected_seq_num);
                sendto(sock, response, strlen(response), 0, (struct sockaddr *)&address, addr_len);
            }
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;

    // Create the UDP socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Start receiving frames and sending responses
    receiver_nack(server_fd);

    close(server_fd);
    return 0;
}