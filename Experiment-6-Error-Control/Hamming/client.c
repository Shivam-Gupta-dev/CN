#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001
#define MAX_DATA 256
#define MAX_CODEWORD 300

static int is_bits(const char *s)
{
    size_t i;
    if (*s == '\0')
        return 0;
    for (i = 0; s[i] != '\0'; i++)
        if (s[i] != '0' && s[i] != '1')
            return 0;
    return 1;
}

static int hamming_bits(const char *data, char *codeword)
{
    int data_len = (int)strlen(data);
    int parity = 0;
    int total;
    int data_index;
    int position;

    while ((1 << parity) < data_len + parity + 1)
        parity++;
    total = data_len + parity;

    for (position = 1; position <= total; position++)
        codeword[position - 1] = '0';

    data_index = 0;
    for (position = 1; position <= total; position++)
        if ((position & (position - 1)) != 0)
            codeword[position - 1] = data[data_index++];

    for (position = 1; position <= total; position <<= 1) {
        int parity_value = 0;
        int covered;
        for (covered = 1; covered <= total; covered++)
            if ((covered & position) != 0 && codeword[covered - 1] == '1')
                parity_value ^= 1;
        codeword[position - 1] = parity_value ? '1' : '0';
    }
    codeword[total] = '\0';
    return total;
}

static int send_line(int socket_fd, const char *line)
{
    char message[MAX_CODEWORD + 32];
    int length = snprintf(message, sizeof(message), "%s\n", line);
    int sent = 0;
    if (length < 0 || length >= (int)sizeof(message))
        return 0;
    while (sent < length) {
        int count = (int)send(socket_fd, message + sent, length - sent, 0);
        if (count <= 0)
            return 0;
        sent += count;
    }
    return 1;
}

static int receive_line(int socket_fd, char *line, int capacity)
{
    int index = 0;
    char character;
    while (index < capacity - 1) {
        int count = (int)recv(socket_fd, &character, 1, 0);
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

int main(void)
{
    int socket_fd;
    int error_position;
    char data[MAX_DATA];
    char codeword[MAX_CODEWORD];
    char response[512];
    struct sockaddr_in server;

    printf("Hamming Code Client\nEnter data bits: ");
    if (scanf("%255s", data) != 1 || !is_bits(data)) {
        printf("Data must contain only 0 and 1.\n");
        return 1;
    }
    if (hamming_bits(data, codeword) >= MAX_CODEWORD) {
        printf("Data is too long.\n");
        return 1;
    }
    printf("Generated codeword: %s\n", codeword);
    printf("Enter bit position to flip (1-%d, 0 for none): ",
           (int)strlen(codeword));
    if (scanf("%d", &error_position) != 1 ||
        error_position < 0 || error_position > (int)strlen(codeword)) {
        printf("Invalid error position.\n");
        return 1;
    }
    if (error_position != 0)
        codeword[error_position - 1] =
            codeword[error_position - 1] == '0' ? '1' : '0';
    printf("Transmitted codeword: %s\n", codeword);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) != 1 ||
        connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(socket_fd);
        return 1;
    }
    if (!send_line(socket_fd, codeword) ||
        !receive_line(socket_fd, response, sizeof(response))) {
        perror("Communication failed");
        close(socket_fd);
        return 1;
    }
    printf("Server: %s\n", response);
    close(socket_fd);
    return 0;
}
