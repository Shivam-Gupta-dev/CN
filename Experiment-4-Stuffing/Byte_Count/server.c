#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_FRAME_SIZE 120
#define MAX_MSG_SIZE 100

int stringLength(char str[])
{
    int length = 0;

    while (str[length] != '\0')
        length++;

    return length;
}

void clearMemory(void *address, int size)
{
    char *bytes = (char *)address;

    for (int i = 0; i < size; i++)
        bytes[i] = 0;
}

int stringToNumber(char str[], int end)
{
    int number = 0;

    for (int i = 0; i < end; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return -1;

        number = number * 10 + (str[i] - '0');
    }

    return number;
}

int separateFrame(char frame[], char msg[], int *byteCount)
{
    int separatorIndex = -1;
    int msgIndex = 0;

    for (int i = 0; frame[i] != '\0'; i++)
    {
        if (frame[i] == '|')
        {
            separatorIndex = i;
            break;
        }
    }

    if (separatorIndex == -1)
        return 0;

    *byteCount = stringToNumber(frame, separatorIndex);
    if (*byteCount < 0)
        return 0;

    for (int i = separatorIndex + 1; frame[i] != '\0'; i++)
        msg[msgIndex++] = frame[i];

    msg[msgIndex] = '\0';

    return msgIndex == *byteCount;
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    clearMemory(&serverAddress, sizeof(serverAddress));
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

    char frame[MAX_FRAME_SIZE];
    char msg[MAX_MSG_SIZE];
    int byteCount = 0;

    int n = recv(clientSocket, frame, sizeof(frame) - 1, 0);
    if (n <= 0)
    {
        perror("Receive failed");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    frame[n] = '\0';

    printf("Received frame: %s\n", frame);

    if (separateFrame(frame, msg, &byteCount))
    {
        printf("Byte count: %d\n", byteCount);
        printf("Message: %s\n", msg);
    }
    else
    {
        printf("Invalid frame or byte count mismatch.\n");
    }

    char ack[] = "OK";
    send(clientSocket, ack, stringLength(ack) + 1, 0);

    close(clientSocket);
    close(serverSocket);

    return 0;
}
