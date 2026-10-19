#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

int main() {
    int s, ns;
    s = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    int option = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    bind(s, (sockaddr*)&server, sizeof(server));
    listen(s, 1);

    cout << "Waiting for client...\n";

    socklen_t len = sizeof(client);
    ns = accept(s, (sockaddr*)&client, &len);

    cout << "Client connected.\n";

    int n;
    recv(ns, &n, sizeof(n), 0);

    int a[10][10];
    recv(ns, a, sizeof(a), 0);

    if (n < 1 || n > 10) {
        const char *message = "Invalid number of routers.\n";
        send(ns, message, strlen(message) + 1, 0);
        close(ns);
        close(s);
        return 0;
    }

    int d[10][10];
    int next[10][10];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            d[i][j] = (i == j) ? 0 : a[i][j];
            next[i][j] = (i != j && a[i][j] < 999) ? j : -1;
        }
    }

    for (int round = 0; round < n - 1; round++) {
        bool changed = false;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
                    if (d[i][k] < 999 && a[i][j] < 999 &&
                        d[i][k] + a[k][j] < d[i][j]) {
                        d[i][j] = d[i][k] + a[k][j];
                        next[i][j] = (i == k) ? j : next[i][k];
                        changed = true;
                    }
                }
            }
        }
        if (!changed)
            break;
    }

    char out[10000];
    string x = "";

    for (int i = 0; i < n; i++) {
        x += "\nRouting Table for Router ";
        x += to_string(i);
        x += "\n";

        x += "Destination\tCost\tNext Hop\n";

        for (int j = 0; j < n; j++) {
            x += to_string(j);
            x += "\t\t";

            if (d[i][j] >= 999) {
                x += "INF\t-\n";
            } else {
                x += to_string(d[i][j]);
                x += "\t";

                if (i == j)
                    x += "-\n";
                else
                    x += to_string(next[i][j]) + "\n";
            }
        }
    }

    strcpy(out, x.c_str());

    send(ns, out, strlen(out) + 1, 0);

    close(ns);
    close(s);

    return 0;
}