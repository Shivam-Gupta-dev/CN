#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_MSG_SIZE 100
#define MAX_STUFFED_SIZE 200
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

void removeNewline(char str[])
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            return;
        }
    }
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

void copyCharacters(char destination[], int start, char source[], int sourceLength)
{
    for (int i = 0; i < sourceLength; i++)
        destination[start + i] = source[i];
}

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

    char msg[MAX_MSG_SIZE];
    int arr[MAX_MSG_SIZE];
    int id = 0;

    printf("Enter message: ");
    if (fgets(msg, sizeof(msg), stdin) == NULL)
    {
        printf("failed to read message\n");
        close(clientSocket);
        return 1;
    }

    removeNewline(msg);
    int msgLength = stringLength(msg);

    for (int i = 0; i <= msgLength - FLAG_LEN; i++)
    {
        if (matchesAt(msg, i, FLAG, FLAG_LEN))
        {
            arr[id++] = i;
        }
    }

    char stuffedMsg[MAX_STUFFED_SIZE];
    int stuffedIndex = 0;
    int arrIndex = 0;

    if (stuffedIndex + FLAG_LEN >= MAX_STUFFED_SIZE)
    {
        printf("stuffed message is too large\n");
        close(clientSocket);
        return 1;
    }

    copyCharacters(stuffedMsg, stuffedIndex, FLAG, FLAG_LEN);
    stuffedIndex += FLAG_LEN;

    for (int i = 0; i < msgLength; i++)
    {
        if (arrIndex < id && arr[arrIndex] == i)
        {
            if (stuffedIndex + ESC_LEN >= MAX_STUFFED_SIZE)
            {
                printf("stuffed message is too large\n");
                close(clientSocket);
                return 1;
            }

            copyCharacters(stuffedMsg, stuffedIndex, ESC, ESC_LEN);
            stuffedIndex += ESC_LEN;
            arrIndex++;
        }

        if (stuffedIndex + 1 >= MAX_STUFFED_SIZE)
        {
            printf("stuffed message is too large\n");
            close(clientSocket);
            return 1;
        }

        stuffedMsg[stuffedIndex++] = msg[i];
    }

    if (stuffedIndex + FLAG_LEN >= MAX_STUFFED_SIZE)
    {
        printf("stuffed message is too large\n");
        close(clientSocket);
        return 1;
    }

    copyCharacters(stuffedMsg, stuffedIndex, FLAG, FLAG_LEN);
    stuffedIndex += FLAG_LEN;
    stuffedMsg[stuffedIndex] = '\0';

    printf("Original message: %s\n", msg);
    printf("Stuffed message: %s\n", stuffedMsg);

    send(clientSocket, stuffedMsg, stringLength(stuffedMsg) + 1, 0);
    char ackMsg[10];
    int n = recv(clientSocket, ackMsg, sizeof(ackMsg) - 1, 0);
    if (n > 0)
    {
        ackMsg[n] = '\0';
        if (n >= 2 && matchesAt(ackMsg, 0, "OK", 2))
            printf("Acknowledgment received.");
    }
    close(clientSocket);

    return 0;
}
