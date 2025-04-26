// receiver.c
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

int init_socket() {
    int sockfd;
    struct sockaddr_in recv_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(SERVER_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    return sockfd;
}

uint8_t process_frame(Frame *frame, uint8_t expected_seq) {
    printf("[RCV] Got Seq=%d, %d bytes\n", frame->seq, frame->len);
    
    if (frame->seq == expected_seq) {
        // Deliver to application
        frame->data[frame->len] = '\0';
        printf("    Delivered: \"%s\"\n", frame->data);
        return expected_seq;
    } else {
        // Out-of-order: re-ACK last in-order
        uint8_t last_ack = (expected_seq - 1);
        printf("    Out-of-order (exp=%d), re-ACK %d\n", expected_seq, last_ack);
        return last_ack;
    }
}

// Send ACK back to sender
void send_ack(int sockfd, struct sockaddr_in *sender_addr, socklen_t sender_len, uint8_t ack_num) {
    Ack ack;
    ack.ack = ack_num;
    
    // Set port for ACK
    sender_addr->sin_port = htons(ACK_PORT);
    
    sendto(sockfd, &ack, sizeof(ack), 0,
           (struct sockaddr*)sender_addr, sender_len);
}

int main() {
    int sockfd = init_socket();
    uint8_t expected_seq = 0;
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    
    printf("---- Go-Back-N Receiver ----\n");

    while (1) {
        Frame frame;
        int n = recvfrom(sockfd, &frame, sizeof(frame), 0,
                     (struct sockaddr*)&sender_addr, &sender_len);
                     
        if (n < (int)(sizeof(uint8_t) + sizeof(uint16_t)))
            continue;

        // Process frame and get ACK number
        uint8_t ack_num = process_frame(&frame, expected_seq);
        
        // Update expected sequence if frame was in order
        if (ack_num == expected_seq)
            expected_seq++;
        
        // Send ACK back
        send_ack(sockfd, &sender_addr, sender_len, ack_num);
    }

    return 0;
}
