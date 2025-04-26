#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <errno.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char* argv[]) {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in serv_addr;

    if (argc < 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Failed to create raw socket");
        exit(1);
    }

    printf("********* RAW SOCKET **********\n");
    printf("********* CLIENT **********\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    while (1) {
        bzero(buffer, sizeof(buffer));

        struct icmphdr *icmp = (struct icmphdr*) buffer;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = getpid();
        icmp->un.echo.sequence = 1;
        strcpy(buffer + sizeof(struct icmphdr), "hello");  // mock data
        icmp->checksum = checksum(icmp, sizeof(struct icmphdr) + strlen("hello"));

        int n = sendto(sockfd, buffer, sizeof(struct icmphdr) + strlen("hello"), 0,
                       (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (n < 0) {
            perror("Send failed");
            exit(1);
        }

        printf("Sent message to server\n");

        if (strncmp("bye", buffer + sizeof(struct icmphdr), 3) == 0) {
            printf("Goodbye to server.\n");
            break;
        }

        sleep(1); // To simulate delay
    }

    close(sockfd);
    return 0;
}

