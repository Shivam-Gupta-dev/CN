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

int required_host_bits(int hosts) {
    int bits = 1;
    unsigned long long capacity = 2;

    if (hosts < 1)
        return -1;

    while (capacity - 2 < (unsigned long long)hosts && bits < 31) {
        capacity = capacity * 2;
        bits++;
    }

    if (capacity - 2 < (unsigned long long)hosts)
        return -1;

    return bits;
}

void sort_requirements(int hosts[MAX_REQUIREMENTS], int count) {
    int i;
    int j;
    int largest;
    int temporary;

    for (i = 0; i < count - 1; i++) {
        largest = i;
        for (j = i + 1; j < count; j++) {
            if (hosts[j] > hosts[largest])
                largest = j;
        }
        temporary = hosts[i];
        hosts[i] = hosts[largest];
        hosts[largest] = temporary;
    }
}

void calculate(struct request *request, struct response *response) {
    unsigned long long base;
    unsigned long long limit;
    unsigned long long current;
    unsigned long long block_size;
    int hosts[MAX_REQUIREMENTS];
    int host_bits;
    int i;

    response->valid = 0;
    response->count = 0;

    if (!valid_address(request->address) || request->prefix < 0 || request->prefix > 30 || request->requirement_count < 1 || request->requirement_count > MAX_REQUIREMENTS)
        return;

    for (i = 0; i < request->requirement_count; i++) {
        if (request->hosts[i] < 1)
            return;
        hosts[i] = request->hosts[i];
    }

    sort_requirements(hosts, request->requirement_count);
    base = make_address(request->address);
    block_size = power_two(32 - request->prefix);
    base = (base / block_size) * block_size;
    limit = base + block_size - 1;
    current = base;

    for (i = 0; i < request->requirement_count; i++) {
        host_bits = required_host_bits(hosts[i]);
        if (host_bits < 2 || host_bits > 32 - request->prefix)
            return;

        block_size = power_two(host_bits);
        if (current % block_size != 0)
            current = current + block_size - (current % block_size);

        if (current + block_size - 1 > limit)
            return;

        response->allocation[i].requested_hosts = hosts[i];
        response->allocation[i].prefix = 32 - host_bits;
        response->allocation[i].network = current;
        response->allocation[i].first = current + 1;
        response->allocation[i].last = current + block_size - 2;
        response->allocation[i].broadcast = current + block_size - 1;
        current = current + block_size;
    }

    response->valid = 1;
    response->count = request->requirement_count;
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
    printf("Waiting for variable-length subnetting client...\n");
    client_length = sizeof(client);
    client_fd = accept(socket_fd, (struct sockaddr *)&client, &client_length);
    recv(client_fd, &request, sizeof(request), 0);
    calculate(&request, &response);
    send(client_fd, &response, sizeof(response), 0);
    close(client_fd);
    close(socket_fd);
    return 0;
}
