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

    struct sockaddr_in clientAddress, serverAddress;

    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0)
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

    if (bind(clientSocket, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0)
    {
        perror("Binding failed");
        close(clientSocket);
        return 1;
    }

    printf("Binding successful\n");

    int addr_len = sizeof(serverAddress);
    int count = recvfrom(clientSocket, serverResponse, sizeof(serverResponse) - 1, 0, (struct sockaddr *)&serverAddress, (socklen_t *)&addr_len);

    printf("The reply from the server is:%s\n", serverResponse);

    close(clientSocket);
    return 0;
}