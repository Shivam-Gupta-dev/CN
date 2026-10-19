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

int parse_ip_cidr(char input[], int address[4], int *prefix) {
    int octet = 0;
    int value = 0;
    int digits = 0;
    int i = 0;

    while (input[i] != '\0' && input[i] != '\n' && input[i] != '/') {
        if (input[i] >= '0' && input[i] <= '9') {
            value = value * 10 + input[i] - '0';
            digits++;
        } else if (input[i] == '.' && digits > 0 && octet < 3) {
            if (value > 255)
                return 0;
            address[octet++] = value;
            value = 0;
            digits = 0;
        } else {
            return 0;
        }
        i++;
    }

    if (octet != 3 || digits == 0 || value > 255)
        return 0;
    address[3] = value;

    if (input[i] == '/') {
        i++;
        value = 0;
        digits = 0;
        while (input[i] >= '0' && input[i] <= '9') {
            value = value * 10 + input[i] - '0';
            digits++;
            i++;
        }
        if (digits == 0 || value > 32 || (input[i] != '\0' && input[i] != '\n'))
            return 0;
        *prefix = value;
    } else {
        if (input[i] != '\0' && input[i] != '\n')
            return 0;
        *prefix = -1;
    }

    return 1;
}

int main(void) {
	int socket_fd;
	int address[4];
	int prefix;
	char input[40];
	struct sockaddr_in server;
	struct ip_result result;

	printf("Enter IP address (CIDR optional, for example 192.168.1.10/24): ");
	scanf("%39s", input);
	if (!parse_ip_cidr(input, address, &prefix)) {
	    printf("Invalid IP/CIDR notation\n");
	    return 0;
	}

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	server.sin_addr.s_addr = htonl(0x7f000001);

	connect(socket_fd, (struct sockaddr *)&server, sizeof(server));
	struct {
	    int address[4];
	    int prefix;
	} request;
	request.address[0] = address[0];
	request.address[1] = address[1];
	request.address[2] = address[2];
	request.address[3] = address[3];
	request.prefix = prefix;
	send(socket_fd, &request, sizeof(request), 0);
	recv(socket_fd, &result, sizeof(result), 0);

	if (result.class_name == 'I') {
		printf("Invalid IP address\n");
		close(socket_fd);
		return 0;
	}

	printf("Class: %c\n", result.class_name);
	printf("Number of bits in NID: %d\n", result.nid_bits);
	printf("Number of bits in HID: %d\n", result.hid_bits);
	printf("Number of networks: %llu\n", result.networks);
	printf("Number of hosts per network: %llu\n", result.hosts);
	printf("Network address: %d.%d.%d.%d\n", result.network[0], result.network[1], result.network[2], result.network[3]);
	printf("First usable address: %d.%d.%d.%d\n", result.first_usable[0], result.first_usable[1], result.first_usable[2], result.first_usable[3]);
	printf("Last usable address: %d.%d.%d.%d\n", result.last_usable[0], result.last_usable[1], result.last_usable[2], result.last_usable[3]);
	printf("Directed broadcast address: %d.%d.%d.%d\n", result.broadcast[0], result.broadcast[1], result.broadcast[2], result.broadcast[3]);

	close(socket_fd);
	return 0;
}
