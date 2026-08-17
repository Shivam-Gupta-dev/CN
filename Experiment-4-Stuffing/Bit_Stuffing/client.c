#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BITS 100
#define MAX_STUFFED_BITS 250
#define MAX_FINAL_FRAME 1000
#define FRAME_SEPARATOR '#'
#define FLAG "01111110"
#define FLAG_LEN 8

struct frames
{
    char frame[MAX_STUFFED_BITS];
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

int isValidBitString(char bits[])
{
    for (int i = 0; bits[i] != '\0'; i++)
    {
        if (bits[i] != '0' && bits[i] != '1')
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

int bitStuff(char bits[], char stuffedBits[])
{
    int stuffedIndex = 0;
    int oneCount = 0;

    copyCharacters(stuffedBits, &stuffedIndex, FLAG, FLAG_LEN);

    for (int i = 0; bits[i] != '\0'; i++)
    {
        if (stuffedIndex + FLAG_LEN + 2 >= MAX_STUFFED_BITS)
            return 0;

        stuffedBits[stuffedIndex++] = bits[i];

        if (bits[i] == '1')
            oneCount++;
        else
            oneCount = 0;

        if (oneCount == 5)
        {
            stuffedBits[stuffedIndex++] = '0';
            oneCount = 0;
        }
    }

    copyCharacters(stuffedBits, &stuffedIndex, FLAG, FLAG_LEN);
    stuffedBits[stuffedIndex] = '\0';

    return 1;
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
    int frameCount;
    scanf("%d", &frameCount);
    struct frames f[frameCount];
    getchar();

    for (int i = 0; i < frameCount; i++)
    {
        char bits[MAX_BITS];
        char stuffedBits[MAX_STUFFED_BITS];

        printf("Enter bits: ");
        if (fgets(bits, sizeof(bits), stdin) == NULL)
        {
            printf("failed to read bits\n");
            close(clientSocket);
            return 1;
        }

        removeNewline(bits);

        if (!isValidBitString(bits))
        {
            printf("Enter only 0 and 1.\n");
            close(clientSocket);
            return 1;
        }

        if (!bitStuff(bits, stuffedBits))
        {
            printf("stuffed bits are too large\n");
            close(clientSocket);
            return 1;
        }

        int j = 0;
        copyCharacters(f[i].frame, &j, stuffedBits, stringLength(stuffedBits) + 1);

        printf("Original bits: %s\n", bits);
        printf("Stuffed bits: %s\n", stuffedBits);
    }

    char finalFrame[MAX_FINAL_FRAME];
    int finalIndex = 0;
    for (int i = 0; i < frameCount; i++)
    {
        copyCharacters(finalFrame, &finalIndex, f[i].frame, stringLength(f[i].frame));
        /*
        if (i != frameCount - 1)
            finalFrame[finalIndex++] = FRAME_SEPARATOR;
        */
    }
    finalFrame[finalIndex] = '\0';

    printf("Final frame: %s\n", finalFrame);
    send(clientSocket, finalFrame, stringLength(finalFrame) + 1, 0);

    char ack[10];
    int receivedBits = recv(clientSocket, ack, sizeof(ack) - 1, 0);
    if (receivedBits > 0)
    {
        ack[receivedBits] = '\0';
        if (receivedBits >= 2 && matchesAt(ack, 0, "OK", 2))
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

    char ack[10];
    int receivedBits = recv(clientSocket, ack, sizeof(ack) - 1, 0);
    if (receivedBits > 0)
    {
        ack[receivedBits] = '\0';
        if (receivedBits >= 2 && matchesAt(ack, 0, "OK", 2))
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

        char ack[10];
        int receivedBits = recv(clientSocket, ack, sizeof(ack) - 1, 0);
        if (receivedBits > 0)
        {
            ack[receivedBits] = '\0';
            if (receivedBits >= 2 && matchesAt(ack, 0, "OK", 2))
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
