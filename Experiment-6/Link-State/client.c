#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(s);
        return 1;
    }

    int n;

    printf("Enter number of routers: ");
    if (scanf("%d", &n) != 1 || n < 1 || n > 10) {
        printf("Invalid number of routers.\n");
        close(s);
        return 1;
    }

    int a[10][10];

    printf("Enter cost matrix:\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scanf("%d", &a[i][j]) != 1 || a[i][j] < 0) {
                printf("Invalid link cost.\n");
                close(s);
                return 1;
            }

            if (a[i][j] == 0 && i != j)
                a[i][j] = 999;
        }
    }

    int src;

    printf("Enter source router: ");
    if (scanf("%d", &src) != 1 || src < 0 || src >= n) {
        printf("Invalid source router.\n");
        close(s);
        return 1;
    }

    send(s, &n, sizeof(n), 0);
    send(s, a, sizeof(a), 0);
    send(s, &src, sizeof(src), 0);

    char out[5000];

    recv(s, out, sizeof(out), 0);

    printf("%s", out);

    close(s);

    return 0;
}