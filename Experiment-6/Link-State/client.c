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

    connect(s, (struct sockaddr *)&server, sizeof(server));

    int n;

    printf("Enter number of routers: ");
    scanf("%d", &n);

    int a[10][10];

    printf("Enter cost matrix:\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%d", &a[i][j]);

            if (a[i][j] == 0 && i != j)
                a[i][j] = 999;
        }
    }

    int src;

    printf("Enter source router: ");
    scanf("%d", &src);

    send(s, &n, sizeof(n), 0);
    send(s, a, sizeof(a), 0);
    send(s, &src, sizeof(src), 0);

    char out[5000];

    recv(s, out, sizeof(out), 0);

    printf("%s", out);

    close(s);

    return 0;
}