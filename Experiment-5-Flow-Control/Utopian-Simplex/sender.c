#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 100

int stringLength(char str[])
{
    int length = 0;

    while (str[length] != '\0')
        length++;

    return length;
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

    // int n;
    // scanf("%d", &n);
    // getchar();
    // send(clientSocket,&n,sizeof(n),0);
    // for (int i = 0; i < n; i++)
    while (true)
    {
        char msg[MAX_MSG_SIZE];
        fgets(msg, MAX_MSG_SIZE, stdin);
        send(clientSocket, msg, stringLength(msg) + 1, 0);
    }

    close(clientSocket);

    return 0;
}
