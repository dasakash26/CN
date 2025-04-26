#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE 1024
#define SERVER_PORT 12345

typedef struct {
    int seq;
    char data[MAX_DATA_SIZE];
} Frame;

typedef struct {
    int ack;
} Ack;

void DeliverData(const char* data) {
    printf("[Receiver] Delivered to application layer: \"%s\"\n", data);
}

void Receiver() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    Frame frame;
    Ack ack;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return;
    }

    printf("[Receiver] Listening on port %d...\n", SERVER_PORT);

    // Loop to accept multiple connections
    while (1) {
        addr_len = sizeof(server_addr);
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        int n = recv(client_fd, &frame, sizeof(Frame), 0);
        if (n > 0) {
            printf("[Receiver] Received frame with seq: %d, data: \"%s\"\n", frame.seq, frame.data);
            DeliverData(frame.data);

            // Send ACK
            ack.ack = frame.seq;
            send(client_fd, &ack, sizeof(Ack), 0);
            printf("[Receiver] Sent ACK: %d\n", ack.ack);
        }

        close(client_fd); // ready for next sender connection
    }

    close(server_fd);
}

int main() {
    printf("---- Stop-and-Wait Protocol Receiver (Noiseless TCP) ----\n\n");
    Receiver();
    return 0;
}

