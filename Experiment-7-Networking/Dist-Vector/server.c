#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_ROUTERS 10
#define INF 999
#define OUTPUT_SIZE 10000

static int receive_all(int socket_fd, void *buffer, size_t size)
{
    size_t received = 0;

    while (received < size)
    {
        ssize_t count = recv(socket_fd, (char *)buffer + received,
                             size - received, 0);
        if (count <= 0)
            return 0;
        received += (size_t)count;
    }
    return 1;
}

static int send_all(int socket_fd, const char *buffer, size_t size)
{
    size_t sent = 0;

    while (sent < size)
    {
        ssize_t count = send(socket_fd, buffer + sent, size - sent, 0);
        if (count <= 0)
            return 0;
        sent += (size_t)count;
    }
    return 1;
}

static void calculate_routes(const int graph[MAX_ROUTERS][MAX_ROUTERS],
                             int n, int distance[MAX_ROUTERS][MAX_ROUTERS],
                             int next_hop[MAX_ROUTERS][MAX_ROUTERS])
{
    int i;
    int j;
    int k;
    int round;

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            distance[i][j] = (i == j) ? 0 : graph[i][j];
            next_hop[i][j] =
                (i == j || graph[i][j] >= INF) ? -1 : j;
            next_hop[i][j] = (graph[i][j]!=0) ? j : -1;
        }
    }

    for (round = 0; round < n - 1; round++)
    {
        int changed = 0;

        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                if (i == j) continue;
                for (k = 0; k < n; k++)
                {
                    if (i == k || j==k) continue;
                    if (distance[i][j] >= INF || distance[j][k] >= INF)
                    continue;
                    if (distance[i][j] + distance[j][k] < distance[i][k])
                    {
                        distance[i][k]=graph[i][j]+distance[j][k];
                        next_hop[i][k]=j;
                        changed=1;
                    }
                    // if (graph[i][k] >= INF || distance[k][j] >= INF)
                    //     continue;
                    // if (graph[i][k] + distance[k][j] < distance[i][j])
                    // {
                    //     distance[i][j] = graph[i][k] + distance[k][j];
                    //     next_hop[i][j] = next_hop[i][k];
                    //     changed = 1;
                    // }
                }
            }
        }
        if (!changed)
            break;
    }
}

static int format_routes(const int distance[MAX_ROUTERS][MAX_ROUTERS],
                         const int next_hop[MAX_ROUTERS][MAX_ROUTERS],
                         int n, char *output, size_t output_size)
{
    size_t used = 0;
    int i;
    int j;

    used += (size_t)snprintf(output + used, output_size - used,
                             "\nDISTANCE VECTOR ROUTING\n"
                             "========================\n");
    for (i = 0; i < n && used < output_size; i++)
    {
        used += (size_t)snprintf(
            output + used, output_size - used,
            "\nRouting Table for Router %d\n"
            "Destination\tCost\tNext Hop\n",
            i);

        for (j = 0; j < n && used < output_size; j++)
        {
            if (distance[i][j] >= INF)
            {
                used += (size_t)snprintf(output + used, output_size - used,
                                         "%d\t\tINF\t-\n", j);
            }
            else if (i == j)
            {
                used += (size_t)snprintf(output + used, output_size - used,
                                         "%d\t\t%d\t-\n", j, distance[i][j]);
            }
            else
            {
                used += (size_t)snprintf(
                    output + used, output_size - used, "%d\t\t%d\t%d\n", j,
                    distance[i][j], next_hop[i][j]);
            }
        }
    }

    if (used >= output_size)
        return 0;
    return (int)used;
}

int main(void)
{
    int server_fd;
    int client_fd;
    int option = 1;
    int n;
    int graph[MAX_ROUTERS][MAX_ROUTERS];
    int distance[MAX_ROUTERS][MAX_ROUTERS];
    int next_hop[MAX_ROUTERS][MAX_ROUTERS];
    char output[OUTPUT_SIZE];
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_length = sizeof(client_address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0 ||
        listen(server_fd, 1) < 0)
    {
        perror("Server setup failed");
        close(server_fd);
        return 1;
    }

    printf("Waiting for client on port %d...\n", PORT);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address,
                       &client_length);
    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }
    printf("Client connected.\n");

    if (!receive_all(client_fd, &n, sizeof(n)) ||
        !receive_all(client_fd, graph, sizeof(graph)))
    {
        fprintf(stderr, "Failed to receive network topology.\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    if (n < 1 || n > MAX_ROUTERS)
    {
        const char *message = "Invalid number of routers.\n";
        send_all(client_fd, message, strlen(message));
        close(client_fd);
        close(server_fd);
        return 1;
    }

    calculate_routes(graph, n, distance, next_hop);
    if (format_routes(distance, next_hop, n, output, sizeof(output)) == 0)
    {
        fprintf(stderr, "Routing table output is too large.\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    printf("%s", output);
    if (!send_all(client_fd, output, strlen(output)))
        perror("Failed to send routing tables");

    close(client_fd);
    close(server_fd);
    return 0;
}
