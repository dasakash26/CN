#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

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

void GetData(char* buffer) {
	printf("[Sender] Enter data to send (or type 'exit' to quit): ");
	fgets(buffer, MAX_DATA_SIZE, stdin);
	buffer[strcspn(buffer, "\n")] = '\0'; 
}

Frame MakeFrame(const char* data, int seq) {
	Frame frame;
	frame.seq = seq;
	strncpy(frame.data, data, MAX_DATA_SIZE);
	return frame;
}

int SendFrameOverTCP(Frame frame, const char* ip, int port, int expected_ack) {
	int sockfd;
	struct sockaddr_in server_addr;
	Ack ack;
	int success = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
    	perror("Socket creation failed");
    	return 0;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &server_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    	perror("Connection to server failed");
    	close(sockfd);
    	return 0;
	}

	printf("[Sender] Sending frame with seq: %d, data: \"%s\"\n", frame.seq, frame.data);
	send(sockfd, &frame, sizeof(Frame), 0);

	// Wait for ACK
	int n = recv(sockfd, &ack, sizeof(Ack), 0);
	if (n > 0) {
    	printf("[Sender] Received ACK: %d\n", ack.ack);
    	if (ack.ack == expected_ack) {
        	success = 1;
    	}
	} else {
    	printf("[Sender] ACK not received or corrupted.\n");
	}

	close(sockfd);
	return success;
}

void Sender() {
	char buffer[MAX_DATA_SIZE];
	int seq = 0;
	srand(time(NULL));

	while (1) {
    	GetData(buffer);

    	if (strcmp(buffer, "exit") == 0) {
        	printf("[Sender] Exiting...\n");
        	break;
    	}

    	Frame frame = MakeFrame(buffer, seq);

    	while (1) {
        	int ack_received = SendFrameOverTCP(frame, SERVER_IP, SERVER_PORT, seq);
        	if (ack_received) break;

        	printf("[Sender] Timeout or wrong ACK. Resending frame...\n");
        	sleep(1); // Simulate timeout
    	}

    	printf("[Sender] Frame sent and acknowledged successfully.\n\n");
    	seq = (seq + 1) % 2; // Toggle between 0 and 1
	}
}

int main() {
	printf("---- Stop-and-Wait Sender (Noisy Channel, Loop Mode) ----\n\n");
	Sender();
	return 0;
}

