#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct packet
{
    int sequenceNumber;
    char data[4];
};

int main()
{
    int serverSocket, clientSocket;
    char serverMessage[] = "Hello Client, we are connected now";

    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress,sizeof(serverAddress)) < 0)
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

    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("Client connected successfully.\n");

    char ack[] = "OK";

    struct packet pkts[3];

    size_t total = 0;

    struct packet p[3];
    struct packet pkt;

    for (int i = 0; i < 3; i++)
    {
        size_t total = 0;

        while (total < sizeof(pkt))
        {
            ssize_t n = recv(clientSocket,((char *)&pkt) + total, sizeof(pkt) - total, 0);

            if (n <= 0)
                return 0;

            total += n;
        }

        p[i]=pkt;
        printf("Sequence: %d\n", pkt.sequenceNumber);
        printf("Data: %s\n", pkt.data);

        send(clientSocket, "OK", 2, 0);
    }

    char ans[4]={'0','0','0','\0'};
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<3; j++)
        {
            if (p[i].data[j]=='1')
            {
                ans[j]='1';
                break;
            }
        }
    }
    printf("Received Bit : %s.\n",ans);

    close(clientSocket);
    close(serverSocket);

    return 0;
}