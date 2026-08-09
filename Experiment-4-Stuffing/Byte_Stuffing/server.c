#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 200
#define FLAG "FLAG"
#define ESC "ESC"
#define FLAG_LEN 4
#define ESC_LEN 3

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

int matchesAt(char str[], int start, char pattern[], int patternLength)
{
    for (int i = 0; i < patternLength; i++)
    {
        if (str[start + i] != pattern[i])
            return 0;
    }

    return 1;
}

void copyCharacters(char destination[], int *destinationIndex, char source[], int sourceLength)
{
    for (int i = 0; i < sourceLength; i++)
    {
        destination[*destinationIndex] = source[i];
        (*destinationIndex)++;
    }
}

void removeFlagPatterns(char msg[], char originalMsg[])
{
    int msgLength = stringLength(msg);
    int start = 0;
    int end = msgLength;
    int originalIndex = 0;

    if (msgLength >= FLAG_LEN && matchesAt(msg, 0, FLAG, FLAG_LEN))
        start = FLAG_LEN;

    if (msgLength >= FLAG_LEN && matchesAt(msg, msgLength - FLAG_LEN, FLAG, FLAG_LEN))
        end = msgLength - FLAG_LEN;

    for (int i = start; i < end; i++)
    {
        if (i <= end - ESC_LEN - FLAG_LEN &&
            matchesAt(msg, i, ESC, ESC_LEN) &&
            matchesAt(msg, i + ESC_LEN, FLAG, FLAG_LEN))
        {
            i += ESC_LEN;
            copyCharacters(originalMsg, &originalIndex, FLAG, FLAG_LEN);
            i += FLAG_LEN - 1;
        }
        else if (i <= end - FLAG_LEN && matchesAt(msg, i, FLAG, FLAG_LEN))
        {
            i += FLAG_LEN - 1;
        }
        else
        {
            originalMsg[originalIndex++] = msg[i];
        }
    }

    originalMsg[originalIndex] = '\0';
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

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
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

    char ack[] = "OK";

    char msg[MAX_MSG_SIZE];
    char originalMsg[MAX_MSG_SIZE];
    int n = recv(clientSocket, msg, sizeof(msg) - 1, 0);
    if (n <= 0)
    {
        perror("Receive failed");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    msg[n] = '\0';
    removeFlagPatterns(msg, originalMsg);
    printf("Received Msg: %s\n", msg);
    printf("After skipping FLAG pattern: %s\n", originalMsg);
    send(clientSocket, ack, stringLength(ack) + 1, 0);
    close(clientSocket);
    close(serverSocket);

    return 0;
}
