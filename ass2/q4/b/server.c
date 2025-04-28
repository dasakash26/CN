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
    char data[FRAME_SIZE];
};

void receiver_nack(int sock)
{
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    struct frame recv_frame, send_frame;
    int received[MAX_FRAMES] = {0};
    int expected_seq = 0;

    while (1)
    {
        ssize_t len = recvfrom(sock, &recv_frame, sizeof(recv_frame), MSG_WAITALL, (struct sockaddr *)&address, &addr_len);
        if (len > 0)
        {
            printf("Received frame %d: %s\n", recv_frame.seq_num, recv_frame.data);

            if (recv_frame.seq_num == expected_seq)
            {
                printf("Frame %d accepted\n", recv_frame.seq_num);
                received[recv_frame.seq_num] = 1;
                expected_seq++;

                // Send ACK
                send_frame.seq_num = recv_frame.seq_num;
                sendto(sock, &send_frame, sizeof(send_frame), 0, (struct sockaddr *)&address, addr_len);
                printf("Sent ACK for frame %d\n", recv_frame.seq_num);
            }
            else
            {
                // Send NACK for missing frame
                send_frame.seq_num = -1;
                sendto(sock, &send_frame, sizeof(send_frame), 0, (struct sockaddr *)&address, addr_len);
                printf("Sent NACK (request retransmission) and ignore the frame\n");
            }
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

    receiver_nack(sock);

    close(sock);
    return 0;
}
