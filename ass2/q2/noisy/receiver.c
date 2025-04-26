#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

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
	printf("[Receiver] Delivered data: \"%s\"\n", data);
}

void Receiver() {
	int server_fd, client_fd;
	struct sockaddr_in server_addr;
	Frame frame;
	Ack ack;
	int last_seq_received = -1;

	srand(time(NULL));

	// Create socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
    	perror("Socket creation failed");
    	exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    	perror("Bind failed");
    	close(server_fd);
    	exit(EXIT_FAILURE);
	}

	// Start listening
	if (listen(server_fd, 5) < 0) {
    	perror("Listen failed");
    	close(server_fd);
    	exit(EXIT_FAILURE);
	}

	printf("[Receiver] Listening on port %d...\n", SERVER_PORT);
	int state=1;
	while (1) {
    	client_fd = accept(server_fd, NULL, NULL);
    	if (client_fd < 0) {
        	perror("Accept failed");
        	continue;
    	}

    	int n = recv(client_fd, &frame, sizeof(Frame), 0);
    	if (n <= 0) {
        	perror("Receive failed or connection closed unexpectedly");
        	close(client_fd);
        	continue;
    	}

    	
    	if (frame.seq != last_seq_received) {
    		printf("[Receiver] Received frame with seq: %d\n", frame.seq);
        	
        	last_seq_received = frame.seq;
    	} else {
    		if(state==1)
        	printf("[Receiver] Duplicate frame. Discarded.\n");
        	else
        	printf("[Receiver] Received last frame with seq: %d\n", frame.seq);
    	}

    	// Simulate random ACK loss (30% chance to drop)
    	if (rand() % 100 < 70) {
    		DeliverData(frame.data);
        	ack.ack = frame.seq;
        	send(client_fd, &ack, sizeof(Ack), 0);
        	printf("[Receiver] Sent ACK: %d\n", ack.ack);
        	state=1;
    	} else {
        	printf("[Receiver] Corrupted packet.\n");
        	state=0;
    	}

    	close(client_fd);
	}

	close(server_fd);
}

int main() 
{
	printf("---- Stop-and-Wait Receiver (Noisy Channel, Robust Version) ----\n\n");
	Receiver();
	return 0;
}

