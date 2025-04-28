#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define PORT 8080
#define MAX_BUF 1024
#define WINDOW_SIZE 4
#define FRAME_SIZE 64
#define TIMEOUT 2
#define MAX_FRAMES 10

struct frame {
    int seq_num;
    char data[FRAME_SIZE];
};

// Simulate 20% chance of frame loss
int frame_lost() {
    return rand() % 10 < 2; // 20% chance
}

void sender_nack(int sock, struct sockaddr_in *addr) {
    struct frame sent_frames[MAX_FRAMES];
    int ack_received[MAX_FRAMES] = {0};
    int total_frames = 0;
    char response[MAX_BUF];
    socklen_t addr_len = sizeof(*addr);

    srand(time(NULL)); // Seed for randomness

    // Get input frames from the user
    while (total_frames < MAX_FRAMES) {
        printf("Enter data for frame %d (or -1 to stop): ", total_frames);
        char input[FRAME_SIZE];
        fgets(input, FRAME_SIZE, stdin);

        if (strncmp(input, "-1", 2) == 0) {
            break;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') input[len - 1] = '\0';

        memset(&sent_frames[total_frames], 0, sizeof(struct frame));
        sent_frames[total_frames].seq_num = total_frames;
        strncpy(sent_frames[total_frames].data, input, FRAME_SIZE - 1);

        total_frames++;
    }

    int base = 0;

    // Start Go-Back-N with NACK
    while (base < total_frames) {
        int window_end = (base + WINDOW_SIZE < total_frames) ? base + WINDOW_SIZE : total_frames;

        // Send frames in the current window
        for (int i = base; i < window_end; i++) {
            if (!ack_received[i]) {
                if (frame_lost()) {
                    printf("Simulated loss of frame %d\n", i);
                } else {
                    printf("Sending frame %d: %s\n", sent_frames[i].seq_num, sent_frames[i].data);
                    sendto(sock, &sent_frames[i], sizeof(sent_frames[i]), 0, (struct sockaddr *)addr, addr_len);
                }
            }
        }

        // Wait for ACK or NACK
        struct timeval tv;
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        ssize_t len = recvfrom(sock, response, MAX_BUF, 0, (struct sockaddr *)addr, &addr_len);
        if (len > 0) {
            response[len] = '\0';
            printf("Received response: %s\n", response);

            int seq_num;
            if (sscanf(response, "ACK for %d", &seq_num) == 1) {
                if (seq_num >= base && seq_num < total_frames) {
                    ack_received[seq_num] = 1;

                    // Slide the window base
                    while (ack_received[base] && base < total_frames) {
                        base++;
                    }
                } else {
                    printf("Ignored invalid ACK number: %d\n", seq_num);
                }
            } else if (sscanf(response, "NACK for %d", &seq_num) == 1) {
                printf("NACK received for frame %d. Resending from frame %d\n", seq_num, seq_num);
                base = seq_num; // Adjust the base to retransmit from the requested frame
            } else {
                printf("Malformed response received: %s\n", response);
            }
        } else {
            printf("No response within timeout. Resending from frame %d\n", base);
        }
    }

    printf("All frames acknowledged. Sender done.\n");
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("172.16.5.16");

    sender_nack(sock, &serv_addr);

    close(sock);
    return 0;
}
