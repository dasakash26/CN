#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080
#define FRAME_SIZE 64

struct frame
{
    int seq_num;
    int ack_num;
    char data[FRAME_SIZE];
};

void receiver(int sock)
{
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    struct frame recv_frame, send_frame;
    int expected_seq_num = 0;

    while (1)
    {
        // Clear the receive buffer before receiving
        memset(&recv_frame, 0, sizeof(recv_frame));

        // Receive a frame from the sender
        ssize_t len = recvfrom(sock, &recv_frame, sizeof(recv_frame), 0,
                               (struct sockaddr *)&address, &addr_len);

        if (len > 0)
        {
            // printf("Received %zd bytes out of %zu expected\n", len, sizeof(recv_frame));

            // Print the raw bytes for debugging (optional)
            // printf("Raw data (first 20 bytes): ");
            // unsigned char *raw = (unsigned char *)&recv_frame;
            // for (int i = 0; i < (len < 20 ? len : 20); i++)
            // {
            //     printf("%02x ", raw[i]);
            // }
            // printf("\n");

            // Check if the received frame has the expected sequence number
            if (recv_frame.seq_num == expected_seq_num)
            {
                printf("Received frame %d: %s\n", recv_frame.seq_num, recv_frame.data);

                // Prepare the ACK frame
                memset(&send_frame, 0, sizeof(send_frame)); // Clear the entire frame
                send_frame.seq_num = expected_seq_num;
                send_frame.ack_num = expected_seq_num;
                strcpy(send_frame.data, "ACK"); // Optional: add ACK message

                // Send acknowledgment back to sender
                sendto(sock, &send_frame, sizeof(send_frame), 0,
                       (struct sockaddr *)&address, addr_len);

                // Update the expected sequence number
                expected_seq_num++;
            }
            else
            {
                printf("Received frame %d, but expected frame %d. Ignoring.\n",
                       recv_frame.seq_num, expected_seq_num);

                // Send ACK for the last correctly received frame
                memset(&send_frame, 0, sizeof(send_frame));
                send_frame.seq_num = expected_seq_num - 1;
                send_frame.ack_num = expected_seq_num - 1;
                strcpy(send_frame.data, "ACK");

                sendto(sock, &send_frame, sizeof(send_frame), 0,
                       (struct sockaddr *)&address, addr_len);
            }
        }
        else if (len == 0)
        {
            printf("Connection closed by sender\n");
        }
        else
        {
            perror("Error receiving frame");
        }
    }
}

int main()
{
    int server_fd;
    struct sockaddr_in address;

    // Create the UDP socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        return -1;
    }

    printf("Receiver is running and waiting for frames...\n");

    // Start receiving frames and sending ACKs
    receiver(server_fd);

    close(server_fd);
    return 0;
}