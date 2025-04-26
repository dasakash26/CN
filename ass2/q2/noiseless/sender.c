#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE 1024
#define SERVER_IP "172.18.8.27"
#define SERVER_PORT 12345

typedef struct {
    int seq;
    char data[MAX_DATA_SIZE];
} Frame;

typedef struct {
    int ack;
} Ack;

void WaitForEventSender() {
    printf("[Sender] Waiting for request...\n");
    sleep(3);
}

void GetData(char* buffer) {
    printf("[Sender] Enter data to send (type 'exit' to quit): ");
    fgets(buffer, MAX_DATA_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; 
}

Frame MakeFrame(const char* data, int seq) {
    Frame frame;
    frame.seq = seq;
    strncpy(frame.data, data, MAX_DATA_SIZE);
    return frame;
}

void SendFrameOverTCP(Frame frame, const char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    Ack ack;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sockfd);
        return;
    }

    send(sockfd, &frame, sizeof(Frame), 0);
    printf("[Sender] Sent frame with seq: %d, data: \"%s\"\n", frame.seq, frame.data);

    recv(sockfd, &ack, sizeof(Ack), 0);
    printf("[Sender] Received ACK: %d\n", ack.ack);

    if (ack.ack != frame.seq) {
        printf("[Sender] Unexpected ACK. Resending frame (not needed in noiseless channel).\n");
    }

    close(sockfd);
}

void Sender() {
    char buffer[MAX_DATA_SIZE];
    int seq = 0;

    while (1) {
        WaitForEventSender();
        GetData(buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("[Sender] Exiting...\n");
            break;
        }

        Frame frame = MakeFrame(buffer, seq);
        SendFrameOverTCP(frame, SERVER_IP, SERVER_PORT);
        seq = (seq + 1) % 2; 
    }
}

int main() {
    printf("---- Stop-and-Wait Protocol Sender (TCP) ----\n\n");
    Sender();
    return 0;
}


