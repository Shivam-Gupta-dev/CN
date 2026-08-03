#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serverSocket, clientSocket;

    struct sockaddr_in serverAddress;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Initialize structure
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    // Listen
    if (listen(serverSocket, 3) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for client connection...\n");

    // Accept client
    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully.\n");

    int s;
    if (recv(clientSocket, &s, sizeof(s), 0) <= 0)
    {
        perror("recv failed");
        return 1;
    }
    printf("s = %d\n", s);

    int a[s];
    recv(clientSocket, a, s * sizeof(int), 0);

    printf("Received array: ");
    for (int i = 0; i < s; i++)
        printf("%d ", a[i]);
    printf("\n");

    int ans = 0;
    for (int i = 0; i < s; i++)
        ans += a[i];

    send(clientSocket, &ans, sizeof(ans), 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}