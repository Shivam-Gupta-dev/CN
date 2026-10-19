#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 100
#define ACK "OK"

int stringLength(char str[])
{
    int length = 0;

    while (str[length] != '\0')
        length++;

    return length;
}

int matchesAt(char str[], int start, char pattern[], int patternLength)
{
    for (int i = 0; i < patternLength; i++)
    {
        if (str[start + i] != pattern[i])
            return 0;
    }

    return 1;
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("socket is not created\n");
        return 1;
    }
    printf("socket is created succesfully\n");

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("there was an error in the connection\n");
        close(clientSocket);
        return 1;
    }
    printf("connection is established.\n");

    printf("Enter the no.of frames: ");
    int n;
    scanf("%d", &n);
    getchar();
    send(clientSocket,&n,sizeof(n),0);
    for (int i = 0; i < n; i++)
    {
        char msg[MAX_MSG_SIZE];
        fgets(msg, MAX_MSG_SIZE, stdin);
        send(clientSocket, msg, stringLength(msg) + 1, 0);
        char ack[5];
        int r = recv(clientSocket,ack,sizeof(ack)-1,0);
        ack[r] = '\0';
        if (matchesAt(ack,0,ACK,2))
        {
            printf("Acknowledgement received.\n");
        }
    }

    close(clientSocket);

    return 0;
}
