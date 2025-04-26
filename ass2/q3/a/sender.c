#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define MAX_DATA_SIZE   1024
#define SERVER_IP       "172.18.8.27"
#define SERVER_PORT     12345
#define MAX_SEQ_NUM     3      
#define WINDOW_SIZE     4
#define ERROR_RATE      0.2  // 20% probability of error

typedef struct {
    uint8_t seq;
    uint16_t len;
    uint16_t checksum;
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) Frame;

typedef struct {
    uint8_t ack;
    uint8_t error;
} __attribute__((packed)) Ack;

Frame window[MAX_SEQ_NUM];
int base = 0;
int next_seq = 0;

uint16_t calculate_checksum(const char* data, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += (uint8_t)data[i];
    }
    return ~sum; 
}

void inject_error(char* data, size_t len) {
    if ((double)rand() / RAND_MAX < ERROR_RATE) {
        int pos = rand() % len;
        data[pos] ^= 0x01; 
        printf("[ERROR] Injected error at position %d\n", pos);
    }
}

int connect_to_receiver() {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }
    
    printf("Connected to receiver at %s:%d\n", SERVER_IP, SERVER_PORT);
    return sockfd;
}

int send_frame(int sockfd, Frame *f) {
    f->checksum = calculate_checksum(f->data, f->len);
    inject_error(f->data, f->len);
    
    int sent = send(sockfd, f, sizeof(uint8_t) + sizeof(uint16_t) + 
                    sizeof(uint16_t) + f->len, 0);
    printf("[SND] Sent Seq=%d\n", f->seq);
    return sent;
}

int get_user_input(Frame *f) {
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

int main() {
    srand(time(NULL));
    
    int sockfd = connect_to_receiver();
    struct timeval tv = {.tv_sec = 0, .tv_usec = 500000}; // 500ms timeout
    
    printf("----  TCP Sender ----\n");
    printf("Error rate: %.1f%%\n", ERROR_RATE * 100);

    while (1) {
        // Send frames if window has space
        while ((next_seq + MAX_SEQ_NUM - base) % MAX_SEQ_NUM < WINDOW_SIZE) {
            Frame f;
            if (!get_user_input(&f))
                goto cleanup;
                
            window[next_seq] = f;
            send_frame(sockfd, &f);
            next_seq = (next_seq + 1) % MAX_SEQ_NUM;
        }

        // Set timeout and wait for ACK
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        Ack ack;
        int received = recv(sockfd, &ack, sizeof(ack), 0);
        
        if (received == sizeof(ack)) {
            printf("[SND] Got ACK=%d, Error=%d\n", ack.ack, ack.error);
            
            if (ack.error) {
                // Retransmit the frame with error
                printf("[SND] Retransmitting frame %d\n", ack.ack);
                send_frame(sockfd, &window[ack.ack]);
            } else {
                base = (ack.ack + 1) % MAX_SEQ_NUM;
            }
        } else if (received < 0) {
            // Timeout - retransmit all frames in window
            printf("[SND] Timeout, retransmitting all frames in window\n");
            for (int i = base; i != next_seq; i = (i + 1) % MAX_SEQ_NUM) {
                send_frame(sockfd, &window[i]);
            }
        } else {
            printf("Connection closed by receiver\n");
            break;
        }
    }
    
cleanup:
    close(sockfd);
    return 0;
}
