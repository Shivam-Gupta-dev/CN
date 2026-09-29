#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_REQUIREMENTS 16

struct request {
    int address[4];
    int prefix;
    int requirement_count;
    int hosts[MAX_REQUIREMENTS];
};

struct allocation {
    int requested_hosts;
    int prefix;
    unsigned long long network;
    unsigned long long first;
    unsigned long long last;
    unsigned long long broadcast;
};

struct response {
    int valid;
    int count;
    struct allocation allocation[MAX_REQUIREMENTS];
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
    printf("Enter number of required subnets (maximum %d): ", MAX_REQUIREMENTS);
    scanf("%d", &request.requirement_count);

    if (request.requirement_count > MAX_REQUIREMENTS)
        request.requirement_count = MAX_REQUIREMENTS;

    for (i = 0; i < request.requirement_count; i++) {
        printf("Hosts required for subnet %d: ", i + 1);
        scanf("%d", &request.hosts[i]);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = htonl(0x7f000001);
    connect(socket_fd, (struct sockaddr *)&server, sizeof(server));
    send(socket_fd, &request, sizeof(request), 0);
    recv(socket_fd, &response, sizeof(response), 0);

    if (!response.valid) {
        printf("The requirements cannot fit in the supplied network.\n");
        close(socket_fd);
        return 0;
    }

    for (i = 0; i < response.count; i++) {
        printf("Subnet for %d hosts: /%d | ", response.allocation[i].requested_hosts, response.allocation[i].prefix);
        print_ip(response.allocation[i].network);
        printf(" - ");
        print_ip(response.allocation[i].broadcast);
        printf(" | Usable: ");
        print_ip(response.allocation[i].first);
        printf(" - ");
        print_ip(response.allocation[i].last);
        printf("\n");
    }

    close(socket_fd);
    return 0;
}
