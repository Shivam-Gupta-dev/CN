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
    int clientSocket, serverSocket;
    struct sockaddr_in serverAddress;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf("Socket creation failed.");
        return 0;
    }
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Binding failed.");
        close(serverSocket);
        return 0;
    }
    if (listen(serverSocket,3) < 0)
    {
        printf("Listening failed.");
        close(serverSocket);
        return 0;
    }
    printf("Server is listening.\n");
    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        printf("Connection failed.");
        close(serverSocket);
        return 0;
    }
    printf("Connection esatblished.\n");
    FILE *f = fopen("copy.txt", "wb");
    if (!f)
    {
        printf("File can't be opened.");
        close(serverSocket);
        close(clientSocket);
        return 0;
    }

    size_t n = 0;

    while ((n = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, n, f);
    }
    printf("File recived.\n");
    close(clientSocket);
    close(serverSocket);
    return 0;
}