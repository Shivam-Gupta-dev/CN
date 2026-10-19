#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 8080
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

int main()
{
    int sock;
    struct sockaddr_in server, client;
    socklen_t clientLen = sizeof(client);

    int received[MAX_FRAMES] = {0};
    int delivered[MAX_FRAMES] = {0};
    int nextDeliver = 0;
    int frame;
    long long startTime;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        close(sock);
        return 1;
    }

    startTime = currentTimeMs();

    printf("=================================\n");
    printf("     SELECTIVE REPEAT SERVER\n");
    printf("=================================\n");
    printf("[%.3fs] Waiting for frames...\n\n", elapsedSeconds(startTime));

    while (1)
    {
        int bytes = recvfrom(
            sock,
            &frame,
            sizeof(frame),
            0,
            (struct sockaddr *)&client,
            &clientLen
        );

        if (bytes < 0)
            continue;

        if (frame == -1)
        {
            printf(
                "\n[%.3fs] Transmission completed.\n",
                elapsedSeconds(startTime)
            );
            break;
        }

        if (frame < 0 || frame >= MAX_FRAMES)
            continue;

        printf(
            "[%.3fs] Received Frame %d\n",
            elapsedSeconds(startTime),
            frame
        );

        if (received[frame] == 0)
        {
            received[frame] = 1;

            if (frame == nextDeliver)
            {
                printf(
                    "[%.3fs] Frame %d accepted; ready for in-order delivery\n",
                    elapsedSeconds(startTime),
                    frame
                );
            }
            else
            {
                printf(
                    "[%.3fs] Frame %d accepted and buffered; waiting for Frame %d\n",
                    elapsedSeconds(startTime),
                    frame,
                    nextDeliver
                );
            }
        }
        else
        {
            printf(
                "[%.3fs] Duplicate Frame %d received; ACK will be resent\n",
                elapsedSeconds(startTime),
                frame
            );
        }

        sendto(
            sock,
            &frame,
            sizeof(frame),
            0,
            (struct sockaddr *)&client,
            clientLen
        );

        printf(
            "[%.3fs] ACK %d sent\n",
            elapsedSeconds(startTime),
            frame
        );

        while (
            nextDeliver < MAX_FRAMES &&
            received[nextDeliver] &&
            delivered[nextDeliver] == 0
        )
        {
            delivered[nextDeliver] = 1;

            printf(
                "[%.3fs] Frame %d delivered to upper layer\n",
                elapsedSeconds(startTime),
                nextDeliver
            );

            nextDeliver++;
        }

        printf("\n");
    }

    close(sock);

    return 0;
}
