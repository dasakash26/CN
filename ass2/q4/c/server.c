#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PORT 8080
#define FRAME_SIZE 64
#define WINDOW_SIZE 4
#define MAX_FRAMES 10

struct frame
{
    int seq_num;
    int ack; // Piggybacked ACK
    char data[FRAME_SIZE];
};

void receiver_piggybacked(int sock)
{
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    struct frame recv_frame, send_frames[MAX_FRAMES];
    int received[MAX_FRAMES] = {0};
    int expected_seq = 0;
    int num_frames = 0;

    // Collect frames from user for piggybacking
    printf("Enter data for piggybacking frames (enter -1 to stop):\n");
    while (num_frames < MAX_FRAMES)
    {
        printf("Enter data for frame %d: ", num_frames);
        char input[FRAME_SIZE];
        fgets(input, FRAME_SIZE, stdin);

        // Check if user wants to stop
        if (strncmp(input, "-1", 2) == 0)
        {
            break;
        }

        // Remove newline if present
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
            input[len - 1] = '\0';

        // Initialize the frame
        memset(&send_frames[num_frames], 0, sizeof(struct frame));
        send_frames[num_frames].seq_num = num_frames;
        send_frames[num_frames].ack = -1; // No ACK initially
        strncpy(send_frames[num_frames].data, input, FRAME_SIZE - 1);

        num_frames++;
    }

    printf("Collected %d frames for piggybacking. Now waiting for incoming frames...\n", num_frames);

    int current_piggyback_frame = 0;

    while (1)
    {
        // Clear receive buffer
        memset(&recv_frame, 0, sizeof(recv_frame));

        // Receive frame from sender
        ssize_t len = recvfrom(sock, &recv_frame, sizeof(recv_frame), 0, (struct sockaddr *)&address, &addr_len);

        if (len > 0)
        {
            printf("Received frame %d: %s\n", recv_frame.seq_num, recv_frame.data);

            if (recv_frame.seq_num == expected_seq)
            {
                printf("Frame %d accepted\n", recv_frame.seq_num);
                received[recv_frame.seq_num] = 1;

                // Send piggybacked ACK with data
                if (current_piggyback_frame < num_frames)
                {
                    // Use the next frame from user input and add ACK
                    send_frames[current_piggyback_frame].ack = recv_frame.seq_num;

                    sendto(sock, &send_frames[current_piggyback_frame], sizeof(struct frame), 0,
                           (struct sockaddr *)&address, addr_len);

                    printf("Sent piggybacked ACK for frame %d with data frame %d: %s\n",
                           recv_frame.seq_num, send_frames[current_piggyback_frame].seq_num,
                           send_frames[current_piggyback_frame].data);

                    current_piggyback_frame++;
                }
                else
                {
                    // If we've used all user frames, send just an ACK
                    struct frame ack_frame;
                    memset(&ack_frame, 0, sizeof(ack_frame));
                    ack_frame.seq_num = -1; // No data frame
                    ack_frame.ack = recv_frame.seq_num;
                    strcpy(ack_frame.data, "ACK only - no more data to piggyback");

                    sendto(sock, &ack_frame, sizeof(ack_frame), 0, (struct sockaddr *)&address, addr_len);
                    printf("Sent ACK for frame %d (no more data to piggyback)\n", recv_frame.seq_num);
                }

                expected_seq++;
            }
            else
            {
                printf("Frame %d discarded. Expected %d.\n", recv_frame.seq_num, expected_seq);

                // Send NACK by piggybacking the expected sequence number - 1
                if (current_piggyback_frame < num_frames)
                {
                    send_frames[current_piggyback_frame].ack = expected_seq - 1;

                    sendto(sock, &send_frames[current_piggyback_frame], sizeof(struct frame), 0,
                           (struct sockaddr *)&address, addr_len);

                    printf("Sent piggybacked NACK (expecting frame %d) with data frame %d\n",
                           expected_seq, send_frames[current_piggyback_frame].seq_num);

                    current_piggyback_frame++;
                }
            }
        }
        else
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

    receiver_piggybacked(sock);

    close(sock);
    return 0;
}