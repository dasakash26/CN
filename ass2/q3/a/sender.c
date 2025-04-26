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
#define SERVER_IP       "172.18.8.27"
#define SERVER_PORT     12345
#define ACK_PORT        12346
#define MAX_SEQ_NUM     3      
#define TIMEOUT_USEC    500000
#define WINDOW_SIZE     4   

typedef struct {
    uint8_t seq;
    uint16_t len;
    char data[MAX_DATA_SIZE];
} Frame;

typedef struct {
    uint8_t ack;
} Ack;

// Global variables for window management
Frame window[MAX_SEQ_NUM];
int base = 0;
int next_seq = 0;

// Initialize socket and server address
int init_socket(struct sockaddr_in *server_addr) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr->sin_addr);

    return sockfd;
}

// Set socket timeout
void set_recv_timeout(int sockfd, int usec) {
    struct timeval tv = { .tv_sec = 0, .tv_usec = usec };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

// Send a frame over UDP
int send_frame(int sockfd, Frame *f, struct sockaddr_in *server_addr) {
    int sent = sendto(sockfd, f, sizeof(uint8_t) + sizeof(uint16_t) + f->len, 0,
                     (struct sockaddr*)server_addr, sizeof(*server_addr));
    printf("[SND] Sent Seq=%d\n", f->seq);
    return sent;
}

// Receive ACK
int receive_ack(int sockfd, Ack *ack, struct sockaddr_in *ack_addr, socklen_t *ack_addrlen) {
    int n = recvfrom(sockfd, ack, sizeof(*ack), 0,
                     (struct sockaddr*)ack_addr, ack_addrlen);
    return (n == sizeof(*ack));
}

// Create new frame from user input
int create_frame(Frame *f) {
    char buffer[MAX_DATA_SIZE];
    
    printf("Data to send (or 'exit'): ");
    if (!fgets(buffer, MAX_DATA_SIZE, stdin)) 
        return 0;
        
    buffer[strcspn(buffer, "\n")] = 0;
    if (strcmp(buffer, "exit") == 0) 
        return 0;

    f->seq = next_seq;
    f->len = strlen(buffer);
    memcpy(f->data, buffer, f->len);
    
    return 1;
}

// Retransmit all frames in window
void retransmit_window(int sockfd, struct sockaddr_in *server_addr) {
    printf("[SND] Timeout, retransmitting from %d to %d\n",
           base, (next_seq + MAX_SEQ_NUM - 1) % MAX_SEQ_NUM);
           
    int seq = base;
    while (seq != next_seq) {
        send_frame(sockfd, &window[seq], server_addr);
        seq = (seq + 1) % MAX_SEQ_NUM;
    }
}

// Check if window has space
int window_has_space() {
    return ((next_seq + MAX_SEQ_NUM - base) % MAX_SEQ_NUM < WINDOW_SIZE);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, ack_addr;
    socklen_t ack_addrlen = sizeof(ack_addr);

    sockfd = init_socket(&server_addr);
    printf("---- Go-Back-N Sender ----\n");

    while (1) {
        // Send frames while window has space
        while (window_has_space()) {
            Frame f;
            if (!create_frame(&f))
                exit(0);
                
            window[next_seq] = f;
            send_frame(sockfd, &f, &server_addr);
            next_seq = (next_seq + 1) % MAX_SEQ_NUM;
        }

        // Wait for ACK with timeout
        set_recv_timeout(sockfd, TIMEOUT_USEC);
        Ack ack;
        
        if (receive_ack(sockfd, &ack, &ack_addr, &ack_addrlen)) {
            printf("[SND] Got ACK=%d\n", ack.ack);
            base = (ack.ack + 1) % MAX_SEQ_NUM;
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                retransmit_window(sockfd, &server_addr);
            } else {
                perror("recvfrom");
                exit(1);
            }
        }
    }
    
    return 0;
}
