#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9002
#define MAX_BITS 256

static int receive_line(int fd, char *line, int capacity)
{
    int index = 0;
    char character;
    while (index < capacity - 1) {
        int count = (int)recv(fd, &character, 1, 0);
        if (count <= 0)
            return 0;
        if (character == '\n') {
            line[index] = '\0';
            return 1;
        }
        line[index++] = character;
    }
    return 0;
}

static int send_line(int fd, const char *line)
{
    char message[512];
    int length = snprintf(message, sizeof(message), "%s\n", line);
    int sent = 0;
    if (length < 0 || length >= (int)sizeof(message))
        return 0;
    while (sent < length) {
        int count = (int)send(fd, message + sent, length - sent, 0);
        if (count <= 0)
            return 0;
        sent += count;
    }
    return 1;
}

static int is_bits(const char *s)
{
    int i;
    if (*s == '\0')
        return 0;
    for (i = 0; s[i] != '\0'; i++)
        if (s[i] != '0' && s[i] != '1')
            return 0;
    return 1;
}

static void divide(char *bits, const char *polynomial)
{
    int length = (int)strlen(bits);
    int degree = (int)strlen(polynomial);
    int i;
    for (i = 0; i <= length - degree; i++)
        if (bits[i] == '1') {
            int j;
            for (j = 0; j < degree; j++)
                bits[i + j] = bits[i + j] == polynomial[j] ? '0' : '1';
        }
}

int main(void)
{
    int server_fd;
    int client_fd;
    int option = 1;
    char frame[MAX_BITS];
    char polynomial[MAX_BITS];
    char remainder[MAX_BITS];
    char response[512];
    struct sockaddr_in address;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(server_fd, 1) < 0) {
        perror("Server setup failed");
        close(server_fd);
        return 1;
    }
    printf("CRC server listening on port %d...\n", PORT);
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }
    if (!receive_line(client_fd, frame, sizeof(frame)) ||
        !receive_line(client_fd, polynomial, sizeof(polynomial)) ||
        !is_bits(frame) || !is_bits(polynomial) ||
        polynomial[0] != '1' || strlen(polynomial) < 2 ||
        strlen(frame) < strlen(polynomial)) {
        snprintf(response, sizeof(response), "Invalid CRC frame or polynomial.");
    } else {
        strcpy(remainder, frame);
        divide(remainder, polynomial);
        if (strspn(remainder + strlen(frame) - strlen(polynomial) + 1, "0") ==
            strlen(polynomial) - 1)
            snprintf(response, sizeof(response),
                     "No error detected. Remainder is zero.");
        else
            snprintf(response, sizeof(response),
                     "Error detected. Remainder: %s",
                     remainder + strlen(frame) - strlen(polynomial) + 1);
    }
    send_line(client_fd, response);
    close(client_fd);
    close(server_fd);
    return 0;
}
