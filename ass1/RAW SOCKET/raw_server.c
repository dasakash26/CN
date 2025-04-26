#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addrlen;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Failed to create raw socket");
        exit(1);
    }

    printf("********* RAW SOCKET **********\n");
    printf("********* SERVER **********\n");

    while (1) {
        bzero(buffer, sizeof(buffer));
        addrlen = sizeof(client_addr);

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addrlen);
        if (n < 0) {
            perror("Receive failed");
            exit(1);
        }

        struct iphdr *ip = (struct iphdr*) buffer;
        struct icmphdr *icmp = (struct icmphdr*)(buffer + (ip->ihl * 4));

        printf("Received ICMP from %s\n", inet_ntoa(client_addr.sin_addr));
        if (icmp->type == ICMP_ECHO) {
            printf("Client message (ICMP ECHO)\n");

            // Echo reply example (not crafting headers in full)
            printf("Reply sent back using ping tools (simulation).\n");

            if (strncmp("bye", (char*)(buffer + sizeof(struct iphdr) + sizeof(struct icmphdr)), 3) == 0) {
                printf("Goodbye from client.\n");
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}

