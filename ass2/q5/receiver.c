#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT 8080
#define FRAME_SIZE 64
#define MAX_BUFFER 4 // Receiver buffer capacity

struct frame
{
    int seq_num;
    char data[FRAME_SIZE];
};

struct ack_frame
{
    int ack_num;
    int buffer_size; // Available buffer size
};

void receiver_optimized(int sock)
{
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    struct frame recv_frame;
    struct ack_frame ack;
    struct frame buffer[MAX_BUFFER]; // Store actual frames, not just sequence numbers
    int buffer_count = 0;
    int expected_seq = 0;
    int process_counter = 0; // Counter to trigger buffer processing

    while (1)
    {
        // Clear receive buffer
        memset(&recv_frame, 0, sizeof(recv_frame));

        ssize_t len = recvfrom(sock, &recv_frame, sizeof(recv_frame), 0, (struct sockaddr *)&sender_addr, &addr_len);
        if (len > 0)
        {
            printf("Received frame %d: %s\n", recv_frame.seq_num, recv_frame.data);

            if (recv_frame.seq_num == expected_seq)
            {
                if (buffer_count < MAX_BUFFER)
                {
                    // Store the entire frame in buffer
                    memcpy(&buffer[buffer_count], &recv_frame, sizeof(struct frame));
                    buffer_count++;
                    expected_seq++;
                    process_counter++;

                    // Prepare and send acknowledgment
                    ack.ack_num = recv_frame.seq_num;
                    ack.buffer_size = MAX_BUFFER - buffer_count; // Remaining buffer size
                    sendto(sock, &ack, sizeof(ack), 0, (struct sockaddr *)&sender_addr, addr_len);
                    printf("Sent ACK for frame %d, Available Buffer: %d\n", ack.ack_num, ack.buffer_size);

                    // Process buffer after receiving 2 frames or when buffer is getting full
                    if (process_counter >= 2 || buffer_count >= MAX_BUFFER - 1)
                    {
                        printf("\n--- Processing buffer ---\n");
                        for (int i = 0; i < buffer_count; i++)
                        {
                            printf("Processing frame %d: %s\n", buffer[i].seq_num, buffer[i].data);
                            // Here you would do actual processing of the frame data
                        }

                        // Empty the buffer
                        buffer_count = 0;
                        process_counter = 0;
                        printf("Buffer emptied. Available buffer: %d\n", MAX_BUFFER);
                        printf("--- Buffer processing complete ---\n\n");
                    }
                }
                else
                {
                    printf("Buffer full. Frame %d discarded.\n", recv_frame.seq_num);

                    // Force process the buffer when it's full
                    printf("\n--- Processing buffer (forced) ---\n");
                    for (int i = 0; i < buffer_count; i++)
                    {
                        printf("Processing frame %d: %s\n", buffer[i].seq_num, buffer[i].data);
                        // Here you would do actual processing of the frame data
                    }

                    // Empty the buffer
                    buffer_count = 0;
                    process_counter = 0;
                    printf("Buffer emptied. Available buffer: %d\n", MAX_BUFFER);
                    printf("--- Buffer processing complete ---\n\n");

                    // Now try to add the current frame
                    memcpy(&buffer[buffer_count], &recv_frame, sizeof(struct frame));
                    buffer_count++;
                    expected_seq++;
                    process_counter++;

                    // Send updated ACK
                    ack.ack_num = recv_frame.seq_num;
                    ack.buffer_size = MAX_BUFFER - buffer_count;
                    sendto(sock, &ack, sizeof(ack), 0, (struct sockaddr *)&sender_addr, addr_len);
                    printf("Sent ACK for frame %d, Available Buffer: %d\n", ack.ack_num, ack.buffer_size);
                }
            }
            else
            {
                printf("Unexpected frame %d. Expected %d. Discarded.\n", recv_frame.seq_num, expected_seq);
            }
        }
        else if (len < 0)
        {
            perror("Error receiving frame");
        }
    }
}

int main()
{
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed");
        return -1;
    }

    printf("Receiver is running...\n");

    receiver_optimized(sock);

    close(sock);
    return 0;
}