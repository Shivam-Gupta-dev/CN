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

    int d[10][10];
    int next[10][10];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            d[i][j] = a[i][j];

            if (i != j && a[i][j] == 0)
                d[i][j] = 999;

            next[i][j] = j;
        }
    }

    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (d[i][k] + d[k][j] < d[i][j]) {
                    d[i][j] = d[i][k] + d[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
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