#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9002
#define MAX_BITS 2048

static int is_bits(const char *value)
{
    int i;

    if (value[0] == '\0')
        return 0;
    for (i = 0; value[i] != '\0'; i++)
        if (value[i] != '0' && value[i] != '1')
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
    char message[MAX_BITS + 64];
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

static void divide(char *frame, const char *generator)
{
    int frame_length = (int)strlen(frame);
    int generator_length = (int)strlen(generator);
    int i;

    for (i = 0; i <= frame_length - generator_length; i++) {
        int j;
        if (frame[i] != '1')
            continue;
        for (j = 0; j < generator_length; j++)
            frame[i + j] =
                frame[i + j] == generator[j] ? '0' : '1';
    }
}

int main(void)
{
    int server_fd;
    int client_fd;
    int option = 1;
    int frame_length;
    int generator_length;
    char generator[MAX_BITS];
    char frame[MAX_BITS];
    char response[MAX_BITS + 64];
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

    if (!receive_line(client_fd, generator, sizeof(generator)) ||
        !receive_line(client_fd, frame, sizeof(frame)) ||
        !is_bits(generator) || !is_bits(frame) || strlen(generator) < 2 ||
        generator[0] != '1' ||
        generator[strlen(generator) - 1] != '1' ||
        strlen(frame) < strlen(generator)) {
        snprintf(response, sizeof(response), "Invalid generator or frame.");
    } else {
        frame_length = (int)strlen(frame);
        generator_length = (int)strlen(generator);
        divide(frame, generator);
        if (strchr(frame + frame_length - generator_length + 1, '1') ==
            NULL) {
            snprintf(response, sizeof(response),
                     "No error detected. Remainder is zero.");
        } else {
            snprintf(response, sizeof(response),
                     "Error detected. Remainder: %s",
                     frame + frame_length - generator_length + 1);
        }
    }
    send_line(client_fd, response);
    close(client_fd);
    close(server_fd);
    return 0;
}