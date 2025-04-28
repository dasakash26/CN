#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 8080
#define FRAME_SIZE 64
#define MAX_FRAMES 16
#define TIMEOUT 2

struct frame {
    int seq_num;
    char data[FRAME_SIZE];
};

struct ack_frame {
    int ack_num;
    int buffer_size; // Available buffer size at the receiver
};

void sender_optimized(int sock, struct sockaddr_in *addr) {
    struct frame frames[MAX_FRAMES];
    int ack_received[MAX_FRAMES] = {0};
    socklen_t addr_len = sizeof(*addr);
    struct ack_frame ack;

    srand(time(NULL)); // Seed for randomness

    int total_frames = 0;
    while (1) {
        // Allow user to enter data for the next frame
        printf("Enter data for frame %d (or type 'exit' to finish): ", total_frames);
        char input[FRAME_SIZE];
        fgets(input, FRAME_SIZE, stdin);

        // Check for exit condition
        if (strncmp(input, "exit", 4) == 0) {
            break;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') input[len - 1] = '\0';

        // Prepare the frame
        frames[total_frames].seq_num = total_frames;
        strncpy(frames[total_frames].data, input, FRAME_SIZE - 1);
        total_frames++;

        if (total_frames >= MAX_FRAMES) {
            printf("Reached maximum number of frames (%d).\n", MAX_FRAMES);
            break;
        }
    }

    int base = 0;
    int window_size = 1; // Initial window size

    // Start optimized sliding window protocol
    while (base < total_frames) {
        int window_end = (base + window_size < total_frames) ? base + window_size : total_frames;

        // Send frames in the current window
        for (int i = base; i < window_end; i++) {
            if (!ack_received[i]) {
                printf("Sending frame %d: %s\n", frames[i].seq_num, frames[i].data);
                sendto(sock, &frames[i], sizeof(frames[i]), 0, (struct sockaddr *)addr, addr_len);
            }
        }

        // Wait for ACK
        struct timeval tv;
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        ssize_t len = recvfrom(sock, &ack, sizeof(ack), 0, (struct sockaddr *)addr, &addr_len);
        if (len > 0) {
            printf("Received ACK for frame %d, Buffer Size: %d\n", ack.ack_num, ack.buffer_size);

            ack_received[ack.ack_num] = 1;

            // Slide the window base
            while (ack_received[base] && base < total_frames) {
                base++;
            }

            // Dynamically adjust the window size based on the receiver's buffer
            window_size = ack.buffer_size;
            if (window_size <= 0) window_size = 1; // Ensure window size is at least 1
        } else {
            printf("Timeout occurred. Resending unacknowledged frames.\n");
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
    serv_addr.sin_addr.s_addr = inet_addr("100.69.69.69");

    sender_optimized(sock, &serv_addr);

    close(sock);
    return 0;
}
