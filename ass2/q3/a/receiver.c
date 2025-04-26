#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE   1024
#define SERVER_PORT     12345
#define MAX_SEQ_NUM     3      

typedef struct {
    uint8_t seq;
    uint16_t len;
    uint16_t checksum;
    uint8_t parity;    // Added for error correction
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) Frame;

typedef struct {
    uint8_t ack;
    uint8_t error;
} __attribute__((packed)) Ack;

// Simple checksum - sum all bytes and return the 16-bit complement
uint16_t calculate_checksum(const char* data, size_t length) {
    uint16_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += (uint8_t)data[i];
    }
    return ~sum; // Return 1's complement
}

// Calculate simple parity for error detection (even parity)
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

// Attempt to correct errors using parity information
int correct_errors(Frame* f) {
    uint8_t current_parity = calculate_parity(f->data, f->len);
    
    if (current_parity == f->parity) {
        // No error detected by parity check
        return 0;
    }
    
    printf("[RECEIVER] Error detected by parity check\n");
    
    // With simple parity, we can detect errors but not reliably correct them
    // For demonstration, we'll attempt a simple correction by checking each byte
    for (size_t i = 0; i < f->len; i++) {
        // Try flipping each bit to see if it fixes parity
        for (int bit = 0; bit < 8; bit++) {
            f->data[i] ^= (1 << bit);  // Flip the bit
            
            if (calculate_parity(f->data, f->len) == f->parity) {
                printf("[RECEIVER] Error corrected at position %zu, bit %d\n", i, bit);
                return 1;
            }
            
            f->data[i] ^= (1 << bit);  // Restore the bit
        }
    }
    
    return 0; // Error couldn't be corrected
}

int start_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create and setup server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen and accept connection
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Waiting for connection on port %d...\n", SERVER_PORT);
    if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) < 0) {
        perror("accept");
        exit(1);
    }
    
    printf("Connected to %s\n", inet_ntoa(client_addr.sin_addr));
    close(server_fd);
    return client_fd;
}

void send_ack(int sock, uint8_t seq_num, uint8_t has_error) {
    Ack ack = {.ack = seq_num, .error = has_error};
    send(sock, &ack, sizeof(ack), 0);
    printf("[ACK] Sent ACK=%d, Error=%d\n", seq_num, has_error);
}

int main() {
    int client_fd = start_server();
    uint8_t expected_seq = 0;
    
    printf("---- Simplified TCP Receiver ----\n");
    printf("Error correction: ENABLED (parity)\n");

    while (1) {
        // Receive frame
        Frame frame;
        int bytes = recv(client_fd, &frame, sizeof(frame), 0);
        
        if (bytes <= 0) {
            if (bytes == 0) printf("Connection closed by sender\n");
            else perror("recv");
            break;
        }
        
        printf("[RCV] Got frame Seq=%d, %d bytes\n", frame.seq, frame.len);

        // Validate checksum
        uint16_t computed_checksum = calculate_checksum(frame.data, frame.len);
        uint8_t has_error = (computed_checksum != frame.checksum);
        
        if (has_error) {
            printf("    Checksum error detected!\n");
            
            // Try to correct the error
            int corrected = correct_errors(&frame);
            
            if (corrected) {
                // Recalculate checksum after correction
                computed_checksum = calculate_checksum(frame.data, frame.len);
                has_error = (computed_checksum != frame.checksum);
                
                if (!has_error) {
                    printf("    Error successfully corrected!\n");
                } else {
                    printf("    Error correction failed - checksum still invalid\n");
                    send_ack(client_fd, frame.seq, 1);  // Request retransmission
                    continue;
                }
            } else {
                send_ack(client_fd, frame.seq, 1);  // Request retransmission
                continue;
            }
        }
        
        // Process in-sequence frame
        if (frame.seq == expected_seq) {
            // Deliver to application
            frame.data[frame.len] = '\0';
            printf("    Delivered: \"%s\"\n", frame.data);
            send_ack(client_fd, expected_seq, 0);
            expected_seq = (expected_seq + 1) % MAX_SEQ_NUM;
        } else {
            // Out-of-order frame
            uint8_t prev_seq = (expected_seq + MAX_SEQ_NUM - 1) % MAX_SEQ_NUM;
            printf("    Out-of-order frame (expected %d)\n", expected_seq);
            send_ack(client_fd, prev_seq, 0);
        }
    }

    close(client_fd);
    return 0;
}
