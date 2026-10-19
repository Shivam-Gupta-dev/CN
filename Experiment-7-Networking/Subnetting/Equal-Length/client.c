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

void print_subnet_mask(int prefix) {
    unsigned long long mask = prefix == 0 ? 0 : (0xffffffffULL << (32 - prefix));
    print_ip(mask);
}

void print_network_calculation(unsigned long long address, int prefix) {
    unsigned long long mask = prefix == 0 ? 0 : (0xffffffffULL << (32 - prefix));
    unsigned long long network = address & mask;

    printf("\nCalculation:\n");
    printf("IP address   : ");
    print_ip(address);
    printf("\nSubnet mask  : ");
    print_ip(mask);
    printf("\nNetwork      : ");
    print_ip(address);
    printf(" AND ");
    print_ip(mask);
    printf(" = ");
    print_ip(network);
    printf("\n");
}

int read_ip(int address[4]) {
    if (scanf("%d.%d.%d.%d", &address[0], &address[1], &address[2], &address[3]) != 4)
        return 0;

    return address[0] >= 0 && address[0] <= 255 &&
           address[1] >= 0 && address[1] <= 255 &&
           address[2] >= 0 && address[2] <= 255 &&
           address[3] >= 0 && address[3] <= 255;
}

int main(void) {
    int socket_fd;
    int i;
    int query_address[4];
    unsigned long long query;
    int subnet_id;
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

    printf("\nS.No.\tSubnet ID\tSubnet Mask\tNetwork\t\tBroadcast\tUsable Range\n");
    printf("------------------------------------------------------------------------------------------------\n");
    for (i = 0; i < response.count; i++) {
        printf("%d\tSubnet %d\t", i + 1, i + 1);
        print_subnet_mask(response.new_prefix);
        printf("\t");
        print_ip(response.subnet[i].network);
        printf("\t");
        print_ip(response.subnet[i].broadcast);
        printf("\t");
        print_ip(response.subnet[i].first);
        printf(" - ");
        print_ip(response.subnet[i].last);
        printf("\n");
    }

    printf("\nEnter an IP address to find its subnet: ");
    if (!read_ip(query_address)) {
        printf("Invalid IP address\n");
        close(socket_fd);
        return 0;
    }

    query = ((unsigned long long)query_address[0] << 24) |
            ((unsigned long long)query_address[1] << 16) |
            ((unsigned long long)query_address[2] << 8) |
            (unsigned long long)query_address[3];
    subnet_id = 0;

    for (i = 0; i < response.count; i++) {
        if (query >= response.subnet[i].network &&
            query <= response.subnet[i].broadcast) {
            subnet_id = i + 1;
            break;
        }
    }

    printf("\nIP address: ");
    print_ip(query);
    if (subnet_id != 0) {
        printf(" belongs to Subnet %d.\n", subnet_id);
        print_network_calculation(query, response.new_prefix);
    } else {
        printf(" is not in any generated subnet.\n");
    }

    close(socket_fd);
    return 0;
}
