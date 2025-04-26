/* tcp_client.c */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP    "172.16.14.49"
#define SERVER_PORT  27000
#define BUF_SIZE     1024

int main(void) {
    int sock_fd;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    char input[BUF_SIZE];
    ssize_t n;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); return 1; }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock_fd,
                (struct sockaddr*)&serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("Connected to %s:%d\n", SERVER_IP, SERVER_PORT);
    
    while(1) {
        printf("Enter message (type 'exit' to quit): ");
        fgets(input, BUF_SIZE, stdin);
        
        // Remove newline character
        input[strcspn(input, "\n")] = 0;
        
        // Check if user wants to exit
        if (strcmp(input, "exit") == 0) {
            break;
        }
        
        send(sock_fd, input, strlen(input), 0);

        n = recv(sock_fd, buf, BUF_SIZE, 0);
        if (n > 0) {
            printf("Echoed: %.*s\n", (int)n, buf);
        }
    }

    close(sock_fd);
    return 0;
}

