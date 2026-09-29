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

int main(void) {
	int socket_fd;
	int address[4];
	struct sockaddr_in server;
	struct ip_result result;

	printf("Enter IP address: ");
	scanf("%d.%d.%d.%d", &address[0], &address[1], &address[2], &address[3]);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	server.sin_addr.s_addr = htonl(0x7f000001);

	connect(socket_fd, (struct sockaddr *)&server, sizeof(server));
	send(socket_fd, address, sizeof(address), 0);
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
