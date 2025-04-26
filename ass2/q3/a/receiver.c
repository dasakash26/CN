// receiver_gbn_tcp.c
// Go-Back-N receiver over TCP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_DATA_SIZE   1024
#define DATA_PORT       12345
#define ACK_PORT        12346

typedef struct {
    uint8_t seq;
    uint16_t len;
    char data[MAX_DATA_SIZE];
} __attribute__((packed)) Frame;

typedef struct {
    uint8_t ack;
} __attribute__((packed)) Ack;

int make_listener(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 5);
    return sock;
}

// accept one connection and return its fd
int accept_conn(int listener) {
    return accept(listener, NULL, NULL);
}

int main() {
    int data_lst = make_listener(DATA_PORT);
    int ack_lst  = make_listener(ACK_PORT);

    uint8_t expected_seq = 0;
    printf("---- GBN receiver over TCP ----\n");

    while (1) {
        // accept data connection
        int ds = accept_conn(data_lst);
        Frame f;
        ssize_t n = read(ds, &f, sizeof(f));
        close(ds);
        if (n < (ssize_t)(sizeof(uint8_t)+sizeof(uint16_t))) continue;

        printf("[RCV] Got Seq=%d, %d bytes\n", f.seq, f.len);

        Ack ack;
        if (f.seq == expected_seq) {
            // deliver
            f.data[f.len] = '\0';
            printf("    Delivered: \"%s\"\n", f.data);
            ack.ack = expected_seq;
            expected_seq++;
        } else {
            // out-of-order: re-ACK last in-order
            ack.ack = expected_seq-1;
            printf("    Out-of-order (exp=%d), re-ACK %d\n",
                   expected_seq, ack.ack);
        }

        // send ACK back
        int as = accept_conn(ack_lst);
        write(as, &ack, sizeof(ack));
        close(as);
    }

    return 0;
}
