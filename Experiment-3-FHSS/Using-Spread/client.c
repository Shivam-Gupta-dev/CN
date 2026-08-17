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
    char a[4];
    printf("Enter the 3 bit: ");
    fgets(a, 4, stdin);
    int clientSocket;
    struct sockaddr_in serveraddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
        printf("socket is not created\n");
    else
        printf("socket is created succesfully\n");

    memset(&serveraddress, 0, sizeof(serveraddress));
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    struct packet p[3];
    for (int i = 0; i < 3; i++)
    {
        p[i].sequenceNumber = i + 1;
        strcpy(p[i].data,"000");
        p[i].data[i] = a[i] == '1' ? '1' : '0';
    }

    int constatus = connect(clientSocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress));

    if (constatus == -1)
        printf("there was an error in the connection\n");
    else
        printf("connection is established.\n");

    // int freq[7] = {100, 200, 300, 400, 500, 600, 700};

    char ack[10];

    for (int i = 0; i < 3; i++)
    {
        send(clientSocket, &p[i], sizeof(p[i]), 0);

        while (1)
        {
            int bytesReceived = recv(clientSocket,ack,sizeof(ack) - 1,0);

            ack[bytesReceived] = '\0';

            if (strcmp(ack, "OK") == 0) break;

            send(clientSocket, &p[i], sizeof(p[i]), 0);
        }
    }

    close(clientSocket);

    return 0;
}
