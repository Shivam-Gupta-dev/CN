#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 8080
#define TIMEOUT_SEC 2
#define MAX_FRAMES 100

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

int shouldDropInChannel(int frame, int lostFrames[], int lossDone[], int lossCount)
{
    int i;

    for (i = 0; i < lossCount; i++)
    {
        if (lostFrames[i] == frame && lossDone[i] == 0)
        {
            lossDone[i] = 1;
            return 1;
        }
    }

    return 0;
}

int main()
{
    int sock;
    struct sockaddr_in server;
    socklen_t serverLen = sizeof(server);

    int totalFrames;
    int windowSize;
    int lossCount;
    int lostFrames[MAX_FRAMES];
    int lossDone[MAX_FRAMES] = {0};

    int base = 0;
    int nextFrame = 0;
    long long startTime;

    printf("=================================\n");
    printf("        GO-BACK-N CLIENT\n");
    printf("=================================\n");

    printf("Enter number of frames: ");
    scanf("%d", &totalFrames);

    if (totalFrames > MAX_FRAMES)
        totalFrames = MAX_FRAMES;

    printf("Enter window size: ");
    scanf("%d", &windowSize);

    printf("Enter number of frames to lose: ");
    scanf("%d", &lossCount);

    if (lossCount < 0)
        lossCount = 0;
    if (lossCount > MAX_FRAMES)
        lossCount = MAX_FRAMES;

    printf("Enter frame numbers to lose: ");
    for (int i = 0; i < lossCount; i++)
    {
        scanf("%d", &lostFrames[i]);

        if (lostFrames[i] < 0 || lostFrames[i] >= totalFrames)
            lostFrames[i] = -1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0)
    {
        printf("Invalid server address\n");
        close(sock);
        return 1;
    }

    {
        struct timeval timeout;

        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        setsockopt(
            sock,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &timeout,
            sizeof(timeout));
    }

    startTime = currentTimeMs();

    printf("\n[%.3fs] Client started.\n\n", elapsedSeconds(startTime));

    while (base < totalFrames)
    {
        while (
            nextFrame < base + windowSize &&
            nextFrame < totalFrames)
        {
            printf(
                "[%.3fs] Sender sends Frame %d (window: %d to %d)\n",
                elapsedSeconds(startTime),
                nextFrame,
                base,
                base + windowSize - 1);

            if (shouldDropInChannel(nextFrame, lostFrames, lossDone, lossCount))
            {
                printf(
                    "[%.3fs] Channel drops Frame %d; sender will know only after timeout\n",
                    elapsedSeconds(startTime),
                    nextFrame);
            }
            else
            {
                sendto(
                    sock,
                    &nextFrame,
                    sizeof(nextFrame),
                    0,
                    (struct sockaddr *)&server,
                    sizeof(server));
            }

            nextFrame++;
        }

        {
            int ack;

            int bytes = recvfrom(
                sock,
                &ack,
                sizeof(ack),
                0,
                (struct sockaddr *)&server,
                &serverLen);

            if (bytes >= 0)
            {
                if (ack >= base)
                {
                    int oldBase = base;

                    base = ack + 1;

                    printf(
                        "[%.3fs] ACK %d received; window slides from %d to %d\n",
                        elapsedSeconds(startTime),
                        ack,
                        oldBase,
                        base);
                }
                else
                {
                    printf(
                        "[%.3fs] Duplicate/old ACK %d received; window stays at %d\n",
                        elapsedSeconds(startTime),
                        ack,
                        base);
                }
            }
            else
            {
                printf(
                    "\n[%.3fs] TIMEOUT for Frame %d\n",
                    elapsedSeconds(startTime),
                    base);
                printf(
                    "[%.3fs] Go-Back-N retransmitting from Frame %d\n\n",
                    elapsedSeconds(startTime),
                    base);

                nextFrame = base;
            }
        }
    }

    {
        int endSignal = -1;

        sendto(
            sock,
            &endSignal,
            sizeof(endSignal),
            0,
            (struct sockaddr *)&server,
            sizeof(server));
    }

    printf(
        "\n[%.3fs] All frames transmitted successfully.\n",
        elapsedSeconds(startTime));

    close(sock);

    return 0;
}
