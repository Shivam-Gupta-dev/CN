#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int socket_fd;
    int n;
    int matrix[10][10];
    struct sockaddr_in server;
    char output[10000];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(socket_fd);
        return 1;
    }

    printf("Enter number of routers (1-10): ");
    if (scanf("%d", &n) != 1 || n < 1 || n > 10) {
        printf("Invalid number of routers.\n");
        close(socket_fd);
        return 1;
    }

    printf("Enter the cost matrix (0 means no direct link):\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scanf("%d", &matrix[i][j]) != 1 || matrix[i][j] < 0) {
                printf("Invalid link cost.\n");
                close(socket_fd);
                return 1;
            }
            if (i != j && matrix[i][j] == 0)
                matrix[i][j] = 999;
        }
    }

    send(socket_fd, &n, sizeof(n), 0);
    send(socket_fd, matrix, sizeof(matrix), 0);

    memset(output, 0, sizeof(output));
    recv(socket_fd, output, sizeof(output) - 1, 0);
    printf("%s", output);

    close(socket_fd);
    return 0;
}
