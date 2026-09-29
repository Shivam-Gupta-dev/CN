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

unsigned long long power_two(int bits) {
    unsigned long long value = 1;
    int i;

    for (i = 0; i < bits; i++)
        value = value * 2;

    return value;
}

unsigned long long make_address(int address[4]) {
    return ((unsigned long long)address[0] << 24) | ((unsigned long long)address[1] << 16) | ((unsigned long long)address[2] << 8) | (unsigned long long)address[3];
}

int valid_address(int address[4]) {
    int i;

    for (i = 0; i < 4; i++) {
        if (address[i] < 0 || address[i] > 255)
            return 0;
    }

    return 1;
}

void calculate(struct request *request, struct response *response) {
    unsigned long long base;
    unsigned long long block_size;
    unsigned long long subnet_count;
    unsigned long long network;
    unsigned long long host_count;
    int i;

    response->valid = 0;
    response->count = 0;

    if (!valid_address(request->address) || request->prefix < 0 || request->prefix > 30 || request->borrowed_bits < 0 || request->prefix + request->borrowed_bits > 30)
        return;

    base = make_address(request->address);
    host_count = 32 - request->prefix;
    block_size = power_two(host_count);
    base = (base / block_size) * block_size;
    subnet_count = power_two(request->borrowed_bits);
    block_size = block_size / subnet_count;

    response->valid = 1;
    response->new_prefix = request->prefix + request->borrowed_bits;
    response->subnet_count = subnet_count;
    response->hosts_per_subnet = block_size - 2;
    response->count = subnet_count < MAX_SUBNETS ? (int)subnet_count : MAX_SUBNETS;

    network = base;
    for (i = 0; i < response->count; i++) {
        response->subnet[i].network = network;
        response->subnet[i].first = network + 1;
        response->subnet[i].last = network + block_size - 2;
        response->subnet[i].broadcast = network + block_size - 1;
        network = network + block_size;
    }
}

int main(void) {
    int socket_fd;
    int client_fd;
    int option = 1;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t client_length;
    struct request request;
    struct response response;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = htonl(0);
    bind(socket_fd, (struct sockaddr *)&server, sizeof(server));
    listen(socket_fd, 1);
    printf("Waiting for equal-length subnetting client...\n");
    client_length = sizeof(client);
    client_fd = accept(socket_fd, (struct sockaddr *)&client, &client_length);
    recv(client_fd, &request, sizeof(request), 0);
    calculate(&request, &response);
    send(client_fd, &response, sizeof(response), 0);
    close(client_fd);
    close(socket_fd);
    return 0;
}
