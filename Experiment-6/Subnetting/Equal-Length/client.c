#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_SUBNETS 64

struct request {
    int address[4];
    int prefix;
    int borrowed_bits;
};

struct subnet {
    unsigned long long network;
    unsigned long long first;
    unsigned long long last;
    unsigned long long broadcast;
};

struct response {
    int valid;
    int new_prefix;
    unsigned long long subnet_count;
    unsigned long long hosts_per_subnet;
    int count;
    struct subnet subnet[MAX_SUBNETS];
};

void print_ip(unsigned long long address) {
    printf("%llu.%llu.%llu.%llu", (address >> 24) & 255, (address >> 16) & 255, (address >> 8) & 255, address & 255);
}

int main(void) {
    int socket_fd;
    int i;
    struct sockaddr_in server;
    struct request request;
    struct response response;

    printf("Enter network IP: ");
    scanf("%d.%d.%d.%d", &request.address[0], &request.address[1], &request.address[2], &request.address[3]);
    printf("Enter original prefix length: ");
    scanf("%d", &request.prefix);
    printf("Enter borrowed bits: ");
    scanf("%d", &request.borrowed_bits);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = htonl(0x7f000001);
    connect(socket_fd, (struct sockaddr *)&server, sizeof(server));
    send(socket_fd, &request, sizeof(request), 0);
    recv(socket_fd, &response, sizeof(response), 0);

    if (!response.valid) {
        printf("Invalid subnetting input\n");
        close(socket_fd);
        return 0;
    }

    printf("New prefix: /%d\n", response.new_prefix);
    printf("Number of subnets: %llu\n", response.subnet_count);
    printf("Hosts per subnet: %llu\n", response.hosts_per_subnet);

    for (i = 0; i < response.count; i++) {
        printf("Subnet %d: ", i + 1);
        print_ip(response.subnet[i].network);
        printf(" - ");
        print_ip(response.subnet[i].broadcast);
        printf(" | Usable: ");
        print_ip(response.subnet[i].first);
        printf(" - ");
        print_ip(response.subnet[i].last);
        printf("\n");
    }

    close(socket_fd);
    return 0;
}
