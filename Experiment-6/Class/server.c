#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

struct ip_result {
    char class_name;
    int nid_bits;
    int hid_bits;
    unsigned long long networks;
    unsigned long long hosts;
    int network[4];
    int first_usable[4];
    int last_usable[4];
    int broadcast[4];
};

int valid_octets(int address[4]) {
    int i;

    for (i = 0; i < 4; i++) {
        if (address[i] < 0 || address[i] > 255)
            return 0;
    }

    return 1;
}

void calculate_ip_details(int address[4], struct ip_result *result) {
    int host_start;
    int i;

    result->class_name = 'I';

    if (address[0] >= 1 && address[0] <= 126) {
        result->class_name = 'A';
        result->nid_bits = 8;
        result->hid_bits = 24;
        result->networks = 126;
        result->hosts = 16777214;
        host_start = 1;
    } else if (address[0] >= 128 && address[0] <= 191) {
        result->class_name = 'B';
        result->nid_bits = 16;
        result->hid_bits = 16;
        result->networks = 16384;
        result->hosts = 65534;
        host_start = 2;
    } else if (address[0] >= 192 && address[0] <= 223) {
        result->class_name = 'C';
        result->nid_bits = 24;
        result->hid_bits = 8;
        result->networks = 2097152;
        result->hosts = 254;
        host_start = 3;
    } else if (address[0] >= 224 && address[0] <= 239) {
        result->class_name = 'D';
        result->nid_bits = 0;
        result->hid_bits = 0;
        result->networks = 0;
        result->hosts = 0;
        host_start = 4;
    } else {
        result->class_name = 'E';
        result->nid_bits = 0;
        result->hid_bits = 0;
        result->networks = 0;
        result->hosts = 0;
        host_start = 4;
    }

    for (i = 0; i < 4; i++) {
        result->network[i] = address[i];
        result->first_usable[i] = address[i];
        result->last_usable[i] = address[i];
        result->broadcast[i] = address[i];
    }

    if (host_start < 4) {
        for (i = host_start; i < 4; i++) {
            result->network[i] = 0;
            result->first_usable[i] = 0;
            result->last_usable[i] = 255;
            result->broadcast[i] = 255;
        }

        result->first_usable[3] = 1;
        result->last_usable[3] = 254;
    }
}

int main(void) {
    int socket_fd;
    int client_fd;
    int address[4];
    int option = 1;
    struct sockaddr_in server;
    struct sockaddr_in client;
    struct ip_result result;
    socklen_t client_length;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = htonl(0);

    bind(socket_fd, (struct sockaddr *)&server, sizeof(server));
    listen(socket_fd, 1);

    printf("Waiting for client...\n");
    client_length = sizeof(client);
    client_fd = accept(socket_fd, (struct sockaddr *)&client, &client_length);
    recv(client_fd, address, sizeof(address), 0);

    if (valid_octets(address))
        calculate_ip_details(address, &result);
    else
        result.class_name = 'I';

    send(client_fd, &result, sizeof(result), 0);
    close(client_fd);
    close(socket_fd);
    return 0;
}
