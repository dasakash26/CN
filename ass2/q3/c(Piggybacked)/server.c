#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080
#define MAX_BUF 1024
#define FRAME_SIZE 64
#define MAX_RESPONSES 10

struct frame {
    int seq_num;
    char data[FRAME_SIZE];
    int ack_num; // Piggybacked acknowledgment
};

void receiver_piggybacking(int sock, char responses[MAX_RESPONSES][FRAME_SIZE], int total_responses) {
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    struct frame recv_frame, send_frame;
    int expected_seq_num = 0;

    while (1) {
        // Receive a frame from the sender
        ssize_t len = recvfrom(sock, &recv_frame, sizeof(recv_frame), MSG_WAITALL, (struct sockaddr *)&address, &addr_len);
        if (len > 0) {
            // Check if the received frame has the expected sequence number
            if (recv_frame.seq_num == expected_seq_num) {
                printf("Received frame %d: %s (Piggybacked ACK for %d)\n", 
                       recv_frame.seq_num, recv_frame.data, recv_frame.ack_num);

                // Prepare response frame with piggybacked acknowledgment
                if (expected_seq_num < total_responses) {
                    send_frame.seq_num = expected_seq_num;
                    strncpy(send_frame.data, responses[expected_seq_num], FRAME_SIZE - 1);
                    send_frame.ack_num = recv_frame.seq_num; // Piggyback acknowledgment

                    // Send the response frame
                    sendto(sock, &send_frame, sizeof(send_frame), 0, (struct sockaddr *)&address, addr_len);
                    printf("Sent frame %d: %s (Piggybacking ACK for %d)\n", 
                           send_frame.seq_num, send_frame.data, send_frame.ack_num);
                } else {
                    printf("No more responses available for frame %d.\n", recv_frame.seq_num);
                }

                // Update the expected sequence number
                expected_seq_num++;
            } else {
                printf("Unexpected frame %d. Waiting for frame %d.\n", recv_frame.seq_num, expected_seq_num);
            }
        } else {
            perror("Error receiving frame");
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    char responses[MAX_RESPONSES][FRAME_SIZE];
    int total_responses = 0;

    // Pre-enter response data
    printf("Enter up to %d responses for piggybacking (-1 to stop):\n", MAX_RESPONSES);
    for (int i = 0; i < MAX_RESPONSES; i++) {
        printf("Response %d: ", i);
        fgets(responses[i], FRAME_SIZE, stdin);

        // Check if the user wants to stop entering responses
        if (strncmp(responses[i], "-1", 2) == 0) {
            responses[i][0] = '\0'; // Clear the response slot
            break;
        }

        // Remove newline character from input
        size_t len = strlen(responses[i]);
        if (len > 0 && responses[i][len - 1] == '\n') {
            responses[i][len - 1] = '\0';
        }

        total_responses++;
    }

    printf("Responses entered. Receiver is ready to start.\n");

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

    printf("Receiver is running and waiting for frames...\n");

    // Start receiving frames and sending piggybacked responses
    receiver_piggybacking(server_fd, responses, total_responses);

    close(server_fd);
    return 0;
}
