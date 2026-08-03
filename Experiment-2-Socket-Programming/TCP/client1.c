#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int clientSocket;
    char serverResponse[256];

    struct sockaddr_in serverAddress;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    printf("Socket created successfully.\n");

    // Initialize server address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);

    // Replace with the server's IP address
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket,(struct sockaddr *)&serverAddress,sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connection established.\n");

    // Receive message from server
    int bytesReceived = recv(clientSocket, serverResponse,
                             sizeof(serverResponse) - 1, 0);

    if (bytesReceived < 0)
    {
        perror("Receive failed");
    }
    else
    {
        printf("Bytes Received: %d\n", bytesReceived);
        serverResponse[bytesReceived] = '\0';
        printf("Reply from server: %s\n", serverResponse);
    }

    close(clientSocket);

    return 0;
}