#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9001
#define MAX_CODEWORD 300

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

static int send_line(int socket_fd, const char *line)
{
    char message[512];
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

static int check_and_correct(char *codeword)
{
    int length = (int)strlen(codeword);
    int syndrome = 0;
    int position;

    for (position = 1; position <= length; position <<= 1) {
        int parity = 0;
        int covered;
        for (covered = 1; covered <= length; covered++)
            if ((covered & position) != 0 && codeword[covered - 1] == '1')
                parity ^= 1;
        if (parity)
            syndrome += position;
    }
    if (syndrome > length)
        return -1;
    if (syndrome != 0)
        codeword[syndrome - 1] =
            codeword[syndrome - 1] == '0' ? '1' : '0';
    return syndrome;
}

int main(void)
{
    int server_fd;
    int client_fd;
    int option = 1;
    char received[MAX_CODEWORD];
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
    printf("Hamming server listening on port %d...\n", PORT);
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }
    if (!receive_line(client_fd, received, sizeof(received)) ||
        !is_bits(received)) {
        snprintf(response, sizeof(response), "Invalid codeword.");
    } else {
        int position = check_and_correct(received);
        if (position < 0)
            snprintf(response, sizeof(response), "Invalid Hamming codeword.");
        else if (position == 0)
            snprintf(response, sizeof(response),
                     "No error detected. Codeword: %s", received);
        else
            snprintf(response, sizeof(response),
                     "Error at bit position %d corrected. Codeword: %s",
                     position, received);
    }
    send_line(client_fd, response);
    close(client_fd);
    close(server_fd);
    return 0;
}
