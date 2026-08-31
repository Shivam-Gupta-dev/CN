#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#define PORT 9000
#define MAX_MSG_SIZE 100
#define ACK "OK"

struct Frame
{
    int seqNo;
    char data[MAX_MSG_SIZE];
};

struct Acknowledgement
{
    int seqNo;
    char status[3];
};

long long currentTimeMs()
{
    struct timeval now;

    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000LL) + (now.tv_usec / 1000);
}

double elapsedSeconds(long long startTime)
{
    return (currentTimeMs() - startTime) / 1000.0;
}

static void trimNewline(char *text)
{
    size_t length = strlen(text);

    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r'))
    {
        text[length - 1] = '\0';
        length--;
    }
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;
    long long startTime;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("Socket creation failed.\n");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
    {
        printf("Invalid server address.\n");
        close(clientSocket);
        return 1;
    }

    startTime = currentTimeMs();

    printf("=================================\n");
    printf(" STOP-AND-WAIT NOISY CLIENT\n");
    printf("=================================\n");

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("[%.3fs] Connection failed.\n", elapsedSeconds(startTime));
        close(clientSocket);
        return 1;
    }

    printf("[%.3fs] Connection established.\n", elapsedSeconds(startTime));

    int frameCount;
    int timeoutSeconds;

    printf("Enter number of frames: ");
    scanf("%d", &frameCount);
    getchar();

    printf("Enter acknowledgement timeout in seconds: ");
    scanf("%d", &timeoutSeconds);
    getchar();

    if (frameCount <= 0 || timeoutSeconds <= 0)
    {
        printf("[%.3fs] Invalid input.\n", elapsedSeconds(startTime));
        close(clientSocket);
        return 1;
    }

    send(clientSocket, &frameCount, sizeof(frameCount), 0);

    for (int i = 0; i < frameCount; i++)
    {
        struct Frame frame;
        struct Acknowledgement ack;
        bool acknowledged = false;
        int simulateLoss = 0;

        memset(&frame, 0, sizeof(frame));
        frame.seqNo = i;

        printf("Enter message for frame %d: ", i + 1);
        if (fgets(frame.data, MAX_MSG_SIZE, stdin) == NULL)
        {
            snprintf(frame.data, MAX_MSG_SIZE, "Frame-%d", i + 1);
        }
        trimNewline(frame.data);

        printf("Simulate data-frame loss for frame %d? (1 = yes, 0 = no): ", i + 1);
        scanf("%d", &simulateLoss);
        getchar();

        while (!acknowledged)
        {
            printf(
                "[%.3fs] Sender sends Frame %d: %s\n",
                elapsedSeconds(startTime),
                i + 1,
                frame.data
            );

            if (simulateLoss)
            {
                printf(
                    "[%.3fs] Channel drops Frame %d; sender waits for ACK until timeout\n",
                    elapsedSeconds(startTime),
                    i + 1
                );
                simulateLoss = 0;
            }
            else
            {
                send(clientSocket, &frame, sizeof(frame), 0);
            }

            fd_set readSet;
            struct timeval timeout;

            FD_ZERO(&readSet);
            FD_SET(clientSocket, &readSet);

            timeout.tv_sec = timeoutSeconds;
            timeout.tv_usec = 0;

            int activity = select(clientSocket + 1, &readSet, NULL, NULL, &timeout);

            if (activity < 0)
            {
                perror("Error while waiting for acknowledgement");
                close(clientSocket);
                return 1;
            }

            if (activity == 0)
            {
                printf(
                    "[%.3fs] TIMEOUT: ACK for Frame %d not received in %d second(s)\n",
                    elapsedSeconds(startTime),
                    i + 1,
                    timeoutSeconds
                );
                printf(
                    "[%.3fs] Stop-and-Wait retransmits Frame %d\n\n",
                    elapsedSeconds(startTime),
                    i + 1
                );
                continue;
            }

            int received = recv(clientSocket, &ack, sizeof(ack), 0);
            if (received <= 0)
            {
                printf("[%.3fs] Connection closed before ACK was received.\n", elapsedSeconds(startTime));
                close(clientSocket);
                return 1;
            }

            if (ack.seqNo == frame.seqNo && strcmp(ack.status, ACK) == 0)
            {
                acknowledged = true;
                printf(
                    "[%.3fs] ACK received for Frame %d; sender moves to next frame\n\n",
                    elapsedSeconds(startTime),
                    i + 1
                );
            }
            else
            {
                printf(
                    "[%.3fs] Invalid ACK received for Frame %d; retransmitting\n\n",
                    elapsedSeconds(startTime),
                    i + 1
                );
            }
        }
    }

    {
        struct Frame endFrame;

        memset(&endFrame, 0, sizeof(endFrame));
        endFrame.seqNo = -1;
        send(clientSocket, &endFrame, sizeof(endFrame), 0);
    }

    printf(
        "[%.3fs] All frames transmitted successfully.\n",
        elapsedSeconds(startTime)
    );

    close(clientSocket);
    return 0;
}
