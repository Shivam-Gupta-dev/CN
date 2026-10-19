#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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

void sendAck(int clientSocket, int seqNo, long long startTime)
{
    struct Acknowledgement ack;

    memset(&ack, 0, sizeof(ack));
    ack.seqNo = seqNo;
    strcpy(ack.status, ACK);

    send(clientSocket, &ack, sizeof(ack), 0);

    printf(
        "[%.3fs] ACK sent for Frame %d\n",
        elapsedSeconds(startTime),
        seqNo + 1
    );
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress;
    int totalFrames;
    int expectedSeqNo = 0;
    long long startTime;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Binding failed");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 3) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    startTime = currentTimeMs();

    printf("=================================\n");
    printf(" STOP-AND-WAIT NOISY SERVER\n");
    printf("=================================\n");
    printf("[%.3fs] Waiting for client connection...\n", elapsedSeconds(startTime));

    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    printf("[%.3fs] Client connected successfully.\n", elapsedSeconds(startTime));

    int received = recv(clientSocket, &totalFrames, sizeof(totalFrames), 0);
    if (received <= 0)
    {
        printf("[%.3fs] Failed to receive the number of frames.\n", elapsedSeconds(startTime));
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    printf(
        "[%.3fs] Sender will transmit %d frame(s).\n\n",
        elapsedSeconds(startTime),
        totalFrames
    );

    while (1)
    {
        struct Frame frame;
        int r;

        memset(&frame, 0, sizeof(frame));

        r = recv(clientSocket, &frame, sizeof(frame), 0);
        if (r <= 0)
        {
            printf("[%.3fs] Connection closed while receiving frame.\n", elapsedSeconds(startTime));
            break;
        }

        if (frame.seqNo == -1)
        {
            printf(
                "\n[%.3fs] End signal received. Transmission complete.\n",
                elapsedSeconds(startTime)
            );
            break;
        }

        if (frame.seqNo < expectedSeqNo)
        {
            printf(
                "[%.3fs] Duplicate Frame %d received; receiver already accepted it\n",
                elapsedSeconds(startTime),
                frame.seqNo + 1
            );
            sendAck(clientSocket, frame.seqNo, startTime);
            printf("\n");
            continue;
        }

        if (frame.seqNo > expectedSeqNo)
        {
            printf(
                "[%.3fs] Expected Frame %d but received Frame %d; discarded\n\n",
                elapsedSeconds(startTime),
                expectedSeqNo + 1,
                frame.seqNo + 1
            );
            continue;
        }

        printf(
            "[%.3fs] Frame %d received: %s\n",
            elapsedSeconds(startTime),
            frame.seqNo + 1,
            frame.data
        );

        printf(
            "[%.3fs] Frame %d accepted and delivered to upper layer\n",
            elapsedSeconds(startTime),
            frame.seqNo + 1
        );

        int dropAck = 0;
        printf("Simulate ACK loss for frame %d? (1 = yes, 0 = no): ", frame.seqNo + 1);
        scanf("%d", &dropAck);
        getchar();

        if (dropAck == 1)
        {
            printf(
                "[%.3fs] Channel drops ACK for Frame %d; sender will timeout and retransmit\n\n",
                elapsedSeconds(startTime),
                frame.seqNo + 1
            );
        }
        else
        {
            sendAck(clientSocket, frame.seqNo, startTime);
            printf("\n");
        }

        expectedSeqNo++;

        if (expectedSeqNo == totalFrames)
        {
            printf(
                "[%.3fs] All data frames accepted; waiting for sender end signal\n\n",
                elapsedSeconds(startTime)
            );
        }
    }

    close(clientSocket);
    close(serverSocket);
    return 0;
}
