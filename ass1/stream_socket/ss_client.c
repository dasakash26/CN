/* tcp_client.c */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP    "127.0.0.1"
#define SERVER_PORT  8080
#define BUF_SIZE     1024

int main(void) {
    int sock_fd;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    const char *msg = "Hello, TCP server!";
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

    send(sock_fd, msg, strlen(msg), 0);

    n = recv(sock_fd, buf, BUF_SIZE, 0);
    if (n > 0) {
        printf("Echoed: %.*s\n", (int)n, buf);
    }

    close(sock_fd);
    return 0;
}

