#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int serverSocket;
    char serverMessage[] = "Hello Client, we are connected now";
    struct sockaddr_in clientAddress, serverAddress;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0)
        printf("socket is not created\n");
    else
        printf("socket is created succesfully\n");

    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(8000);
    clientAddress.sin_addr.s_addr = INADDR_ANY;

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    printf("Binding successful\n");

    sendto(serverSocket, serverMessage, strlen(serverMessage), 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
    close(serverSocket);

    return 0;
}