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

int shouldDropInChannel(
    int frame,
    int base,
    int lostPositionInWindow,
    int *lastDroppedWindowBase
)
{
    int lostFrameInWindow = base + lostPositionInWindow - 1;

    if (lostPositionInWindow <= 0)
        return 0;

    if (frame == lostFrameInWindow && *lastDroppedWindowBase != base)
    {
        *lastDroppedWindowBase = base;
        return 1;
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
    int lostPositionInWindow;

    int base = 0;
    int nextFrame = 0;
    int totalTransmissions = 0;
    int lastDroppedWindowBase = -1;
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

    if (windowSize <= 0)
        windowSize = 1;
    if (windowSize > totalFrames)
        windowSize = totalFrames;

    printf("Enter packet position to lose in every window (1 to %d, 0 for no loss): ", windowSize);
    scanf("%d", &lostPositionInWindow);

    if (lostPositionInWindow < 0 || lostPositionInWindow > windowSize)
        lostPositionInWindow = 0;

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
            totalTransmissions++;

            printf(
                "[%.3fs] Transmission %d: Sender sends Frame %d (window: %d to %d)\n",
                elapsedSeconds(startTime),
                totalTransmissions,
                nextFrame,
                base,
                base + windowSize - 1);

            if (shouldDropInChannel(
                    nextFrame,
                    base,
                    lostPositionInWindow,
                    &lastDroppedWindowBase))
            {
                printf(
                    "[%.3fs] Channel drops packet position %d of this window: transmission %d carrying Frame %d\n",
                    elapsedSeconds(startTime),
                    lostPositionInWindow,
                    totalTransmissions,
                    nextFrame);
                printf(
                    "[%.3fs] Sender will know only after timeout\n",
                    elapsedSeconds(startTime));
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
        elapsedSeconds(startTime)
    );
    printf("Total transmission attempts: %d\n", totalTransmissions);

    close(sock);

    return 0;
}
