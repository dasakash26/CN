// receiver_gbn.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE   1024
#define SERVER_PORT     12345
#define ACK_PORT        12346

typedef struct {
    uint8_t seq;
    uint16_t len;
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) Frame;

typedef struct {
    uint8_t ack;
} __attribute__((packed)) Ack;

int main() {
    int sockfd;
    struct sockaddr_in recv_addr, sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    // create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    // bind to data port
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(SERVER_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind"); exit(1);
    }

    uint8_t expected_seq = 0;
    printf("---- Go-Back-N Receiver ----\n");

    while (1) {
        Frame f;
        int n = recvfrom(sockfd, &f, sizeof(f), 0,
                         (struct sockaddr*)&sender_addr, &sender_len);
        if (n < (int)(sizeof(uint8_t)+sizeof(uint16_t))) continue;

        // extract payload length & data
        uint16_t len = f.len;
        printf("[RCV] Got Seq=%d, %d bytes\n", f.seq, len);

        Ack ack;
        if (f.seq == expected_seq) {
            // deliver to application
            f.data[len] = '\0';
            printf("    Delivered: \"%s\"\n", f.data);
            // ack this frame
            ack.ack = expected_seq;
            expected_seq++;
        } else {
            // out-of-order: re-ack last in-order
            ack.ack = (expected_seq - 1);
            printf("    Out-of-order (exp=%d), re-ACK %d\n",
                   expected_seq, ack.ack);
        }

        // send ACK back
        sender_addr.sin_port = htons(ACK_PORT);
        sendto(sockfd, &ack, sizeof(ack), 0,
               (struct sockaddr*)&sender_addr, sender_len);
    }

    return 0;
}
