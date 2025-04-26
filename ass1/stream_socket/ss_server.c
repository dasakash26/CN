/* tcp_server.c */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT       8080
#define BACKLOG    5
#define BUF_SIZE   1024

int main(void) {
    int listen_fd, conn_fd;
    struct sockaddr_in addr, client_addr;
    socklen_t client_len;
    char buf[BUF_SIZE];
    ssize_t n;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, BACKLOG) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("TCP server listening on port %d...\n", PORT);

    while (1) {
        client_len = sizeof(client_addr);
        conn_fd = accept(listen_fd,
                         (struct sockaddr*)&client_addr,
                         &client_len);
        if (conn_fd < 0) {
            perror("accept");
            continue;
        }

        n = recv(conn_fd, buf, BUF_SIZE, 0);
        if (n > 0) {
            printf("Received: %.*s\n", (int)n, buf);
            send(conn_fd, buf, n, 0);  /* Echo back */
        }

        close(conn_fd);
    }

    /* unreachable */
    close(listen_fd);
    return 0;
}
