#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9002
#define MAX_BITS 1024
#define MAX_LINE 2048

static int is_bits(const char *value)
{
    size_t i;

    if (value[0] == '\0')
        return 0;
    for (i = 0; value[i] != '\0'; i++)
        if (value[i] != '0' && value[i] != '1')
            return 0;
    return 1;
}

static int polynomial_to_bits(const char *expression, char *bits, int capacity)
{
    char input[MAX_LINE];
    char *term;
    int highest = -1;
    int exponents[MAX_BITS];
    int count = 0;
    int i;

    if (strlen(expression) >= sizeof(input))
        return 0;
    strcpy(input, expression);
    for (term = strtok(input, "+"); term != NULL; term = strtok(NULL, "+")) {
        int exponent;
        char *x = strchr(term, 'x');

        if (x == NULL) {
            if (sscanf(term, "%d", &exponent) != 1 || exponent != 0)
                return 0;
        } else if (x[1] == '\0') {
            exponent = 1;
        } else if (x[1] != '^' || sscanf(x + 2, "%d", &exponent) != 1) {
            return 0;
        }
        if (exponent < 0 || exponent >= MAX_BITS || count >= MAX_BITS)
            return 0;
        exponents[count++] = exponent;
        if (exponent > highest)
            highest = exponent;
    }
    if (count == 0 || highest < 1 || highest + 1 >= capacity)
        return 0;

    for (i = 0; i <= highest; i++)
        bits[i] = '0';
    for (i = 0; i < count; i++)
        bits[highest - exponents[i]] = '1';
    bits[highest + 1] = '\0';
    return 1;
}

static int send_line(int socket_fd, const char *line)
{
    char message[MAX_LINE + 2];
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

static int make_codeword(const char *data, const char *generator,
                         char *codeword, int capacity)
{
    int data_length = (int)strlen(data);
    int generator_length = (int)strlen(generator);
    int codeword_length = data_length + generator_length - 1;
    int i;

    if (codeword_length + 1 > capacity)
        return 0;
    strcpy(codeword, data);
    for (i = data_length; i < codeword_length; i++)
        codeword[i] = '0';
    codeword[codeword_length] = '\0';

    for (i = 0; i < data_length; i++) {
        int j;
        if (codeword[i] != '1')
            continue;
        for (j = 0; j < generator_length; j++)
            codeword[i + j] =
                codeword[i + j] == generator[j] ? '0' : '1';
    }
    for (i = 0; i < codeword_length; i++)
        codeword[i] = codeword[i] == '1' ? '1' : '0';
    for (i = data_length - 1; i >= 0; i--)
        codeword[i] = data[i];
    return 1;
}

int main(void)
{
    int socket_fd;
    int flip_position;
    char data[MAX_BITS];
    char generator_input[MAX_LINE];
    char generator[MAX_BITS];
    char codeword[MAX_BITS * 2];
    char response[MAX_LINE];
    struct sockaddr_in server;

    printf("CRC Client\nEnter data bits: ");
    if (scanf("%1023s", data) != 1 || !is_bits(data)) {
        printf("Data must contain only 0 and 1.\n");
        return 1;
    }
    printf("Enter generator polynomial (binary or x^n notation): ");
    if (scanf("%2047s", generator_input) != 1)
        return 1;
    if (is_bits(generator_input) &&
        strlen(generator_input) < sizeof(generator))
        strcpy(generator, generator_input);
    else if (!polynomial_to_bits(generator_input, generator, sizeof(generator))) {
        printf("Invalid generator polynomial.\n");
        return 1;
    }
    if (strlen(generator) < 2 || generator[0] != '1' ||
        generator[strlen(generator) - 1] != '1') {
        printf("Generator must start and end with 1.\n");
        return 1;
    }
    if (!make_codeword(data, generator, codeword, sizeof(codeword))) {
        printf("Input is too long.\n");
        return 1;
    }
    printf("Generator polynomial: %s\n", generator);
    printf("Transmitted codeword: %s\n", codeword);
    printf("Enter bit position to flip (1-%d, 0 for none): ",
           (int)strlen(codeword));
    if (scanf("%d", &flip_position) != 1 ||
        flip_position < 0 || flip_position > (int)strlen(codeword)) {
        printf("Invalid error position.\n");
        return 1;
    }
    if (flip_position != 0)
        codeword[flip_position - 1] =
            codeword[flip_position - 1] == '0' ? '1' : '0';
    printf("Sent frame: %s\n", codeword);

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
    if (!send_line(socket_fd, generator) || !send_line(socket_fd, codeword) ||
        !receive_line(socket_fd, response, sizeof(response))) {
        perror("Communication failed");
        close(socket_fd);
        return 1;
    }
    printf("Server: %s\n", response);
    close(socket_fd);
    return 0;
}
