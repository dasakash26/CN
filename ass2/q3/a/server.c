#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080
#define MAX_BUF 1024

// Define the frame structure
struct frame {
    int seq_num;           // Sequence number for the frame
    char data[MAX_BUF];    // Frame data (using MAX_BUF for simplicity)
};

void receiver_non_nack(int sock) {
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    struct frame received_frame;
    char ack[MAX_BUF];
    int expected_seq_num = 0; // Start with frame 0

    while (1) {
        // Receive frame from the sender
        ssize_t len = recvfrom(sock, &received_frame, sizeof(received_frame), MSG_WAITALL, (struct sockaddr *)&address, &addr_len);
        if (len > 0) {
            // Check if the received frame has the expected sequence number
            if (received_frame.seq_num == expected_seq_num) {
                // Print the received frame's sequence number and data
                printf("Received frame %d: %s\n", received_frame.seq_num, received_frame.data);

                // Create an ACK message with the sequence number of the received frame
                snprintf(ack, MAX_BUF, "ACK for %d", received_frame.seq_num);

                // Send ACK to the sender with the correct sequence number
                sendto(sock, ack, strlen(ack), 0, (struct sockaddr *)&address, addr_len);
                printf("Sent %s\n", ack);  // Print ACK message sent to sender

                // Update the expected sequence number to the next frame
                expected_seq_num++;
            } else {
                // Frame sequence number mismatch, ignore the frame
                printf("Ignored frame %d: out of sequence (expected %d)\n", received_frame.seq_num, expected_seq_num);
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

    // Set up the server address struct
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Start receiving frames and sending ACKs
    receiver_non_nack(server_fd);

    // Close the socket after finishing
    close(server_fd);
    return 0;
}
