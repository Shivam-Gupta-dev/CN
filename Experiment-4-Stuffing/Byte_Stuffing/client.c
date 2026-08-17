#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_MSG_SIZE 100
#define MAX_STUFFED_SIZE 200
#define MAX_FINAL_FRAME 1000
#define FRAME_SEPARATOR '#'
#define FLAG "FLAG"
#define ESC "ESC"
#define FLAG_LEN 4
#define ESC_LEN 3

struct frames
{
    char frame[MAX_STUFFED_SIZE];
};

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

int byteStuff(char msg[], char stuffedMsg[])
{
    int msgLength = stringLength(msg);
    int stuffedIndex = 0;

    if (stuffedIndex + FLAG_LEN >= MAX_STUFFED_SIZE)
        return 0;

    copyCharacters(stuffedMsg, stuffedIndex, FLAG, FLAG_LEN);
    stuffedIndex += FLAG_LEN;

    for (int i = 0; i < msgLength; i++)
    {
        if (i <= msgLength - FLAG_LEN && matchesAt(msg, i, FLAG, FLAG_LEN))
        {
            if (stuffedIndex + ESC_LEN >= MAX_STUFFED_SIZE)
                return 0;

            copyCharacters(stuffedMsg, stuffedIndex, ESC, ESC_LEN);
            stuffedIndex += ESC_LEN;
        }

        if (stuffedIndex + 1 >= MAX_STUFFED_SIZE)
            return 0;

        stuffedMsg[stuffedIndex++] = msg[i];
    }

    if (stuffedIndex + FLAG_LEN >= MAX_STUFFED_SIZE)
        return 0;

    copyCharacters(stuffedMsg, stuffedIndex, FLAG, FLAG_LEN);
    stuffedIndex += FLAG_LEN;
    stuffedMsg[stuffedIndex] = '\0';

    return 1;
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

    printf("Enter the no.of frames: ");
    int frameCount;
    scanf("%d", &frameCount);
    struct frames f[frameCount];
    getchar();

    for (int i = 0; i < frameCount; i++)
    {
        char msg[MAX_MSG_SIZE];
        char stuffedMsg[MAX_STUFFED_SIZE];

        printf("Enter message: ");
        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            printf("failed to read message\n");
            close(clientSocket);
            return 1;
        }

        removeNewline(msg);

        if (!byteStuff(msg, stuffedMsg))
        {
            printf("stuffed message is too large\n");
            close(clientSocket);
            return 1;
        }

        copyCharacters(f[i].frame, 0, stuffedMsg, stringLength(stuffedMsg) + 1);

        printf("Original message: %s\n", msg);
        printf("Stuffed message: %s\n", stuffedMsg);
    }

    char finalFrame[MAX_FINAL_FRAME];
    int finalIndex = 0;
    for (int i = 0; i < frameCount; i++)
    {
        copyCharacters(finalFrame, finalIndex, f[i].frame, stringLength(f[i].frame));
        finalIndex += stringLength(f[i].frame);
        /*
        if (i != frameCount - 1)
            finalFrame[finalIndex++] = FRAME_SEPARATOR;
        */
    }
    finalFrame[finalIndex] = '\0';

    printf("Final frame: %s\n", finalFrame);
    send(clientSocket, finalFrame, stringLength(finalFrame) + 1, 0);

    char ackMsg[10];
    int receivedBits = recv(clientSocket, ackMsg, sizeof(ackMsg) - 1, 0);
    if (receivedBits > 0)
    {
        ackMsg[receivedBits] = '\0';
        if (receivedBits >= 2 && matchesAt(ackMsg, 0, "OK", 2))
            printf("Acknowledgment received.\n");
    }
    else
    {
        perror("Receive failed");
        close(clientSocket);
        return 1;
    }

    /*
    send(clientSocket, &frameCount, sizeof(frameCount), 0);
    char ackMsg[10];
    int receivedBits = recv(clientSocket, ackMsg, sizeof(ackMsg) - 1, 0);
    if (receivedBits > 0)
    {
        ackMsg[receivedBits] = '\0';
        if (receivedBits >= 2 && matchesAt(ackMsg, 0, "OK", 2))
            printf("Acknowledgment received.\n");
    }
    else
    {
        perror("Receive failed");
        close(clientSocket);
        return 1;
    }

    for (int i = 0; i < frameCount; i++)
    {
        send(clientSocket, f[i].frame, stringLength(f[i].frame) + 1, 0);

        char ackMsg[10];
        int receivedBits = recv(clientSocket, ackMsg, sizeof(ackMsg) - 1, 0);
        if (receivedBits > 0)
        {
            ackMsg[receivedBits] = '\0';
            if (receivedBits >= 2 && matchesAt(ackMsg, 0, "OK", 2))
                printf("Acknowledgment received.\n");
            else
            {
                printf("Acknowledgment not received.\n");
                close(clientSocket);
                return 1;
            }
        }
        else
        {
            perror("Receive failed");
            close(clientSocket);
            return 1;
        }
    }
    */

    close(clientSocket);

    return 0;
}
