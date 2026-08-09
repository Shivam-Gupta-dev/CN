#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MSG_SIZE 100
#define MAX_FRAME_SIZE 120

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

void numberToString(int number, char str[])
{
    int index = 0;
    char reverse[10];

    if (number == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (number > 0)
    {
        reverse[index++] = (number % 10) + '0';
        number = number / 10;
    }

    for (int i = 0; i < index; i++)
        str[i] = reverse[index - i - 1];

    str[index] = '\0';
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

int createFrame(char msg[], char frame[])
{
    int msgLength = stringLength(msg);
    char count[10];
    int frameIndex = 0;

    numberToString(msgLength, count);

    for (int i = 0; count[i] != '\0'; i++)
        frame[frameIndex++] = count[i];

    frame[frameIndex++] = '|';

    for (int i = 0; msg[i] != '\0'; i++)
    {
        if (frameIndex + 1 >= MAX_FRAME_SIZE)
            return 0;

        frame[frameIndex++] = msg[i];
    }

    frame[frameIndex] = '\0';
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

    char msg[MAX_MSG_SIZE];
    char frame[MAX_FRAME_SIZE];

    printf("Enter message or bits: ");
    if (fgets(msg, sizeof(msg), stdin) == NULL)
    {
        printf("failed to read message\n");
        close(clientSocket);
        return 1;
    }

    removeNewline(msg);

    if (!createFrame(msg, frame))
    {
        printf("frame is too large\n");
        close(clientSocket);
        return 1;
    }

    printf("Original message: %s\n", msg);
    printf("Byte count: %d\n", stringLength(msg));
    printf("Frame: %s\n", frame);

    send(clientSocket, frame, stringLength(frame) + 1, 0);

    char ack[10];
    int n = recv(clientSocket, ack, sizeof(ack) - 1, 0);
    if (n > 0)
    {
        ack[n] = '\0';
        if (n >= 2 && matchesAt(ack, 0, "OK", 2))
            printf("Acknowledgment received.\n");
    }

    close(clientSocket);

    return 0;
}
