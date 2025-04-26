// sender_gbn.c
// Go-Back-N sender over UDP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_DATA_SIZE   1024
#define SERVER_IP       "127.0.0.1"
#define SERVER_PORT     12345
#define ACK_PORT        12346
#define MAX_SEQ_NUM     256      // sequence numbers: 0…255
#define TIMEOUT_USEC    500000   // 500 ms

typedef struct {
    uint8_t seq;
    uint16_t len;
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) Frame;

typedef struct {
    uint8_t ack;
} __attribute__((packed)) Ack;

int window_size;
int base = 0;
int next_seq = 0;
Frame window[MAX_SEQ_NUM];

int sockfd;
struct sockaddr_in server_addr, ack_addr;
socklen_t ack_addrlen = sizeof(ack_addr);

// set a timeout on the socket for recvfrom()
void set_recv_timeout(int usec) {
    struct timeval tv = { .tv_sec = 0, .tv_usec = usec };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int udpsend(Frame *f) {
    return sendto(sockfd, f, sizeof(uint8_t)+sizeof(uint16_t)+f->len, 0,
                  (struct sockaddr*)&server_addr, sizeof(server_addr));
}

int udprecv_ack(Ack *ack) {
    int n = recvfrom(sockfd, ack, sizeof(*ack), 0,
                     (struct sockaddr*)&ack_addr, &ack_addrlen);
    return (n == sizeof(*ack));
}

int main() {
    char buffer[MAX_DATA_SIZE];
    printf("Enter window size (e.g. 4): ");
    scanf("%d", &window_size);
    getchar();

    // open one UDP socket for both data and ACKs
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    // server (receiver) data port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("---- Go-Back-N Sender ----\n");

    while (1) {
        // fill window
        while ((next_seq + MAX_SEQ_NUM - base) % MAX_SEQ_NUM < window_size) {
            printf("Data to send (or 'exit'): ");
            if (!fgets(buffer, MAX_DATA_SIZE, stdin)) exit(0);
            buffer[strcspn(buffer, "\n")] = 0;
            if (strcmp(buffer,"exit")==0) exit(0);

            Frame f;
            f.seq = next_seq;
            f.len = strlen(buffer);
            memcpy(f.data, buffer, f.len);
            window[next_seq] = f;

            udpsend(&f);
            printf("[SND] Sent Seq=%d\n", f.seq);
            next_seq = (next_seq+1) % MAX_SEQ_NUM;
        }

        // wait for ACK or timeout
        set_recv_timeout(TIMEOUT_USEC);
        Ack ack;
        if (udprecv_ack(&ack)) {
            printf("[SND] Got ACK=%d\n", ack.ack);
            // slide base to ack+1
            base = (ack.ack + 1) % MAX_SEQ_NUM;
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // timeout: retransmit all in window
                printf("[SND] Timeout, retransmitting from %d to %d\n",
                       base, (next_seq+MAX_SEQ_NUM-1)%MAX_SEQ_NUM);
                int seq = base;
                while (seq != next_seq) {
                    udpsend(&window[seq]);
                    printf("[SND] Resent Seq=%d\n", seq);
                    seq = (seq+1)%MAX_SEQ_NUM;
                }
            } else {
                perror("recvfrom");
                exit(1);
            }
        }
    }
    return 0;
}
