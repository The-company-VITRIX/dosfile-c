#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define PAYLOAD_SIZE 1464

const char *SRC_ADDR = "1.1.1.1";
const char *DST_ADDR = "2.2.2.2";
const int SRC_PORT = 1234;
const int DST_PORT = 4321;

static uint16_t checksum(void *addr, int count) {
    uint16_t *ptr = addr;
    uint32_t sum = 0;

    while (count > 1)  {
        sum += *ptr++;
        count -= 2;
    }

    if (count > 0) {
        sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return ~sum;
}

int main(int argc, char *argv[]) {
    int socket_fd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
    if (socket_fd == -1) {
        perror("socket()");
        return EXIT_FAILURE;
    }

    struct {
        struct ip ip;
        char data[PAYLOAD_SIZE];
    } packet = { 0 };

    struct ip *ip = &packet.ip;
    ip->ip_v = 4;
    ip->ip_hl = 5;
    ip->ip_len = sizeof(packet);
    ip->ip_id = htons(12345);
    ip->ip_off = htons(IP_DF);
    ip->ip_ttl = 64;
    ip->ip_p = IPPROTO_TCP;
    ip->ip_src.s_addr = inet_addr(SRC_ADDR);
    ip->ip_dst.s_addr = inet_addr(DST_ADDR);
    ip->ip_sum = checksum(&packet, ip->ip_len >> 1);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(DST_ADDR);

    while (1) {
        if (sendto(socket_fd, &packet, sizeof(packet), 0, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("sendto");
        }
    }

    return EXIT_SUCCESS;
}
