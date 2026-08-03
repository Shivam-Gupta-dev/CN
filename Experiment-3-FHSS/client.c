#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

struct packet
{
    int sequenceNumber;
    char data[4];
};

int main()
{
    int clientSocket;
    struct sockaddr_in serveraddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
        printf("socket is not created\n");
    else
        printf("socket is created succesfully\n");

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    int constatus = connect(clientSocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress));

    if (constatus == -1)
        printf("there was an error in the connection\n");
    else
        printf("connection is estableshed.\n");

    int freq[7] = {100, 200, 300, 400, 500, 600, 700};

    char ack[10];

    for (int i = 0; i < 7; i++)
    {
        char msg[100];
        snprintf(msg, sizeof(msg), "Sending Data using Frequency: %d.", freq[i]);

        send(clientSocket, msg, strlen(msg) + 1, 0);

        recv(clientSocket, ack, sizeof(ack), 0);
    }

    close(clientSocket);

    return 0;
}