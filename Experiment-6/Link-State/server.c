#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int s, ns;
    s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server, client;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr *)&server, sizeof(server));
    listen(s, 1);

    printf("Waiting for client...\n");

    socklen_t len = sizeof(client);
    ns = accept(s, (struct sockaddr *)&client, &len);

    printf("Client connected.\n");

    int n;
    recv(ns, &n, sizeof(n), 0);

    int a[10][10];

    recv(ns, a, sizeof(a), 0);

    int src;
    recv(ns, &src, sizeof(src), 0);

    int d[10], v[10], p[10];

    for (int i = 0; i < n; i++) {
        d[i] = a[src][i];
        v[i] = 0;
        p[i] = src;
    }

    d[src] = 0;
    v[src] = 1;

    for (int k = 0; k < n - 1; k++) {
        int m = 999, u = -1;

        for (int i = 0; i < n; i++) {
            if (!v[i] && d[i] < m) {
                m = d[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        v[u] = 1;

        for (int i = 0; i < n; i++) {
            if (!v[i] && a[u][i] != 999 &&
                d[u] + a[u][i] < d[i]) {
                d[i] = d[u] + a[u][i];
                p[i] = u;
            }
        }
    }

    char out[5000];
    size_t used = 0;

    used += (size_t)snprintf(out + used, sizeof(out) - used,
                             "\nRouting Table\nDestination\tCost\tNext Hop\n");

    for (int i = 0; i < n; i++) {
        used += (size_t)snprintf(out + used, sizeof(out) - used,
                                 "%d\t\t", i);

        if (d[i] >= 999) {
            used += (size_t)snprintf(out + used, sizeof(out) - used,
                                     "INF\t-\n");
            continue;
        }

        used += (size_t)snprintf(out + used, sizeof(out) - used,
                                 "%d\t", d[i]);

        if (i == src) {
            used += (size_t)snprintf(out + used, sizeof(out) - used,
                                     "-\n");
        } else {
            int h = i;

            while (p[h] != src)
                h = p[h];

            used += (size_t)snprintf(out + used, sizeof(out) - used,
                                     "%d\n", h);
        }
    }

    send(ns, out, strlen(out) + 1, 0);

    close(ns);
    close(s);

    return 0;
}