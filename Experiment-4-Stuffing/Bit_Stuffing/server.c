#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BITS 250
#define MAX_FINAL_FRAME 1000
#define FRAME_SEPARATOR '#'
#define FLAG "01111110"
#define FLAG_LEN 8

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

void bitDestuff(char stuffedBits[], char originalBits[])
{
    int stuffedLength = stringLength(stuffedBits);
    int start = 0;
    int end = stuffedLength;
    int originalIndex = 0;
    int oneCount = 0;

    if (stuffedLength >= FLAG_LEN && matchesAt(stuffedBits, 0, FLAG, FLAG_LEN))
        start = FLAG_LEN;

    if (stuffedLength >= FLAG_LEN && matchesAt(stuffedBits, stuffedLength - FLAG_LEN, FLAG, FLAG_LEN))
        end = stuffedLength - FLAG_LEN;

    for (int i = start; i < end; i++)
    {
        originalBits[originalIndex++] = stuffedBits[i];

        if (stuffedBits[i] == '1')
            oneCount++;
        else
            oneCount = 0;

        if (oneCount == 5 && i + 1 < end && stuffedBits[i + 1] == '0')
        {
            i++;
            oneCount = 0;
        }
    }

    originalBits[originalIndex] = '\0';
}

void separateFinalFrame(char finalFrame[])
{
    int finalFrameLength = stringLength(finalFrame);
    int currentIndex = 0;
    int frameNumber = 1;

    while (currentIndex < finalFrameLength)
    {
        char stuffedBits[MAX_BITS];
        char originalBits[MAX_BITS];
        int frameIndex = 0;
        int frameStart = -1;
        int frameEnd = -1;

        for (int i = currentIndex; i <= finalFrameLength - FLAG_LEN; i++)
        {
            if (matchesAt(finalFrame, i, FLAG, FLAG_LEN))
            {
                frameStart = i;
                break;
            }
        }

        if (frameStart == -1)
            return;

        for (int i = frameStart + FLAG_LEN; i <= finalFrameLength - FLAG_LEN; i++)
        {
            if (matchesAt(finalFrame, i, FLAG, FLAG_LEN))
            {
                frameEnd = i + FLAG_LEN;
                break;
            }
        }

        if (frameEnd == -1)
        {
            printf("Invalid frame: ending flag not found.\n");
            return;
        }

        for (int i = frameStart; i < frameEnd; i++)
        {
            if (frameIndex + 1 >= MAX_BITS)
            {
                printf("Invalid frame: frame is too large.\n");
                return;
            }

            stuffedBits[frameIndex++] = finalFrame[i];
        }

        stuffedBits[frameIndex] = '\0';

        /*
        if (finalFrame[frameStart] == FRAME_SEPARATOR)
            frameStart++;
        */

        bitDestuff(stuffedBits, originalBits);

        printf("Frame %d\n", frameNumber);
        printf("Received bits: %s\n", stuffedBits);
        printf("After removing flag and stuffed bits: %s\n", originalBits);

        currentIndex = frameEnd;
        frameNumber++;
    }
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

    char finalFrame[MAX_FINAL_FRAME];
    int n = recv(clientSocket, finalFrame, sizeof(finalFrame) - 1, 0);
    if (n <= 0)
    {
        perror("Receive failed");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    finalFrame[n] = '\0';
    printf("Received final frame: %s\n", finalFrame);
    separateFinalFrame(finalFrame);

    char ack[] = "OK";
    send(clientSocket, ack, stringLength(ack) + 1, 0);

    /*
    int frameCount;
    int n = recv(clientSocket, &frameCount, sizeof(frameCount), 0);
    if (n <= 0)
    {
        perror("Receive failed");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    char ack[] = "OK";
    send(clientSocket, ack, stringLength(ack) + 1, 0);

    for (int i = 0; i < frameCount; i++)
    {
        char stuffedBits[MAX_BITS];
        char originalBits[MAX_BITS];
        int n = recv(clientSocket, stuffedBits, sizeof(stuffedBits) - 1, 0);
        if (n <= 0)
        {
            perror("Receive failed");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        stuffedBits[n] = '\0';
        bitDestuff(stuffedBits, originalBits);

        printf("Received bits: %s\n", stuffedBits);
        printf("After removing flag and stuffed bits: %s\n", originalBits);

        send(clientSocket, ack, stringLength(ack) + 1, 0);
    }
    */

    close(clientSocket);
    close(serverSocket);

    return 0;
}
