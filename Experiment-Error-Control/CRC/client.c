#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9002
#define MAX_BITS 256

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

static int send_line(int fd, const char *line)
{
    char message[MAX_BITS + 4];
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

int main(void)
{
    int fd;
    int flip;
    char data[MAX_BITS];
    char polynomial[MAX_BITS];
    char frame[MAX_BITS];
    char response[512];
    struct sockaddr_in server;

    printf("CRC Client\nEnter data bits: ");
    if (scanf("%255s", data) != 1 || !is_bits(data)) {
        printf("Data must contain only 0 and 1.\n");
        return 1;
    }
    printf("Enter generator polynomial: ");
    if (scanf("%255s", polynomial) != 1 || !is_bits(polynomial) ||
        polynomial[0] != '1' || strlen(polynomial) < 2) {
        printf("Polynomial must be binary, start with 1, and have at least 2 bits.\n");
        return 1;
    }
    if (strlen(data) + strlen(polynomial) >= MAX_BITS) {
        printf("Input is too long.\n");
        return 1;
    }
    strcpy(frame, data);
    memset(frame + strlen(data), '0', strlen(polynomial) - 1);
    frame[strlen(data) + strlen(polynomial) - 1] = '\0';
    divide(frame, polynomial);
    {
        size_t i;
        for (i = 0; i < strlen(data); i++)
            frame[i] = data[i];
    }
    printf("Transmitted codeword: %s\n", frame);
    printf("Enter bit position to flip (1-%d, 0 for none): ",
           (int)strlen(frame));
    if (scanf("%d", &flip) != 1 || flip < 0 || flip > (int)strlen(frame)) {
        printf("Invalid error position.\n");
        return 1;
    }
    if (flip != 0)
        frame[flip - 1] = frame[flip - 1] == '0' ? '1' : '0';
    printf("Sent frame: %s\n", frame);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) != 1 ||
        connect(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(fd);
        return 1;
    }
    if (!send_line(fd, frame) || !send_line(fd, polynomial) ||
        !receive_line(fd, response, sizeof(response))) {
        perror("Communication failed");
        close(fd);
        return 1;
    }
    printf("Server: %s\n", response);
    close(fd);
    return 0;
}
