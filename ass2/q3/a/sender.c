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
    uint8_t parity;    // Added for error correction
    uint8_t error_bit; // Explicit error bit for testing
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) Frame;

typedef struct {
    uint8_t ack;
    uint8_t error;
} __attribute__((packed)) Ack;

Frame window[MAX_SEQ_NUM];
int base = 0;
int next_seq = 0;
int retransmission_count = 0;

uint16_t calculate_checksum(const char* data, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += (uint8_t)data[i];
    }
    return ~sum; 
}

// Calculate simple parity for error correction (even parity)
uint8_t calculate_parity(const char* data, size_t length) {
    uint8_t parity = 0;
    for (size_t i = 0; i < length; i++) {
        // Count number of 1 bits in each byte
        uint8_t byte = data[i];
        while (byte) {
            parity ^= (byte & 1);
            byte >>= 1;
        }
    }
    return parity;
}

// Simplified error injection - just set the error bit
void inject_error(Frame *frame) {
    if ((double)rand() / RAND_MAX < ERROR_RATE) {
        frame->error_bit = 1;
        printf("[ERROR] Injected error (error_bit set to 1)\n");
    } else {
        frame->error_bit = 0;
    }
}

// Apply error correction data to the frame
void apply_error_correction(Frame* f) {
    f->parity = calculate_parity(f->data, f->len);
    printf("[SENDER] Applied error correction (parity: %d)\n", f->parity);
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
    // Apply error correction before calculating checksum
    apply_error_correction(f);
    
    f->checksum = calculate_checksum(f->data, f->len);
    inject_error(f);
    
    int sent = send(sockfd, f, sizeof(uint8_t) + sizeof(uint16_t) + 
                    sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t) + f->len, 0);
    printf("[SND] Sent Seq=%d%s\n", f->seq, f->error_bit ? " (with error bit set)" : "");
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
    
    printf("---- Simplified TCP Sender ----\n");
    printf("Error rate: %.1f%%\n", ERROR_RATE * 100);
    printf("Error correction: ENABLED (parity)\n");
    printf("Error bit: ENABLED (explicit error flagging)\n");

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
                printf("[SND] Retransmission #%d for frame %d\n", ++retransmission_count, ack.ack);
                
                // Reset error bit before retransmission
                window[ack.ack].error_bit = 0;
                send_frame(sockfd, &window[ack.ack]);
            } else {
                retransmission_count = 0;
                base = (ack.ack + 1) % MAX_SEQ_NUM;
                printf("[SND] Window advanced to base=%d\n", base);
            }
        } else if (received < 0) {
            // Timeout - retransmit all frames in window
            printf("[SND] Timeout, retransmitting all frames in window\n");
            for (int i = base; i != next_seq; i = (i + 1) % MAX_SEQ_NUM) {
                printf("[SND] Retransmitting frame %d after timeout\n", i);
                // Reset error bit before retransmission
                window[i].error_bit = 0;
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
