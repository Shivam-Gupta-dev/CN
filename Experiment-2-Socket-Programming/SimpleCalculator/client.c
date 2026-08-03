#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("socket is not created\n");
        return 0;
    }
    printf("socket is created succesfully\n");

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
    printf("Connection successfull.\n");

    int a[] = {12, 22};
    int element_count = sizeof(a) / sizeof(a[0]);
    send(clientSocket, &element_count, sizeof(element_count), 0);

    send(clientSocket, a, sizeof(a), 0);
    int ans = 0;
    recv(clientSocket, &ans, sizeof(a), 0);
    printf("Ans: %d", ans);
    close(clientSocket);
}