#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <string.h>

int main()
{
    char buffer[4096];
    int clientSocket;
    struct sockaddr_in serverAddress;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("Socket creation failed.");
        return 0;
    }
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Connection failed.");
        close(clientSocket);
        return 0;
    }
    printf("Connection esatblished.\n");
    FILE *f = fopen("temp.txt", "rb");
    if (!f)
    {
        printf("File can't be opened.");
        close(clientSocket);
        return 0;
    }

    size_t n = 0;

    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        size_t total = 0;
        while (total < n)
        {
            int sent = send(clientSocket, buffer + total, n - total, 0);
            total += sent;
        }
    }
    printf("File Sent.\n");
    close(clientSocket);
    return 0;
}