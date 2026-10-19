#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t addrlen = sizeof(client);

    char ip[50];
    char result[5000];

    int a, b, c, d;
    int subnets;

    /* ==============================
       CREATE SOCKET
       ============================== */

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    /* ==============================
       SERVER DETAILS
       ============================== */

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    /* ==============================
       BIND
       ============================== */

    if (bind(server_fd,
             (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    /* ==============================
       LISTEN
       ============================== */

    listen(server_fd, 5);

    printf("=====================================\n");
    printf("         SUBNETTING SERVER\n");
    printf("=====================================\n");

    printf("Waiting for client...\n");

    /* ==============================
       ACCEPT CLIENT
       ============================== */

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client,
                       &addrlen);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(1);
    }

    printf("Client connected.\n");

    /* ==============================
       RECEIVE IP
       ============================== */

    memset(ip, 0, sizeof(ip));

    recv(client_fd,
         ip,
         sizeof(ip),
         0);

    /* ==============================
       RECEIVE NUMBER OF SUBNETS
       ============================== */

    recv(client_fd,
         &subnets,
         sizeof(subnets),
         0);

    printf("IP received: %s\n", ip);
    printf("Number of subnets: %d\n", subnets);

    /* ==============================
       EXTRACT IP OCTETS
       ============================== */

    if (sscanf(ip,
               "%d.%d.%d.%d",
               &a, &b, &c, &d) != 4)
    {
        strcpy(result, "Invalid IP address.");

        send(client_fd,
             result,
             sizeof(result),
             0);

        close(client_fd);
        close(server_fd);

        return 0;
    }

    /* ==============================
       VALIDATE IP
       ============================== */

    if (a < 0 || a > 255 ||
        b < 0 || b > 255 ||
        c < 0 || c > 255 ||
        d < 0 || d > 255)
    {
        strcpy(result, "Invalid IP address.");

        send(client_fd,
             result,
             sizeof(result),
             0);

        close(client_fd);
        close(server_fd);

        return 0;
    }

    /* ==============================
       VALIDATE SUBNET COUNT
       ============================== */

    if (subnets <= 0)
    {
        strcpy(result,
               "Invalid number of subnets.");

        send(client_fd,
             result,
             sizeof(result),
             0);

        close(client_fd);
        close(server_fd);

        return 0;
    }

    /* ==============================
       FIND CLASS
       ============================== */

    char className;
    int networkBits;
    int hostBits;

    if (a >= 1 && a <= 126)
    {
        className = 'A';

        networkBits = 8;
        hostBits = 24;
    }
    else if (a >= 128 && a <= 191)
    {
        className = 'B';

        networkBits = 16;
        hostBits = 16;
    }
    else if (a >= 192 && a <= 223)
    {
        className = 'C';

        networkBits = 24;
        hostBits = 8;
    }
    else
    {
        strcpy(result,
               "Subnetting is applicable only for Class A, B and C.");

        send(client_fd,
             result,
             sizeof(result),
             0);

        close(client_fd);
        close(server_fd);

        return 0;
    }

    /* ==============================
       FIND BORROWED BITS
       ============================== */

    int borrowedBits = 0;
    int totalSubnets = 1;

    while (totalSubnets < subnets)
    {
        totalSubnets = totalSubnets * 2;
        borrowedBits++;
    }

    /* ==============================
       CHECK HOST BITS
       ============================== */

    if (borrowedBits >= hostBits)
    {
        strcpy(result,
               "Number of subnets is too large for this IP class.");

        send(client_fd,
             result,
             sizeof(result),
             0);

        close(client_fd);
        close(server_fd);

        return 0;
    }

    /* ==============================
       NEW PREFIX
       ============================== */

    int newPrefix =
        networkBits + borrowedBits;

    /* ==============================
       REMAINING HOST BITS
       ============================== */

    int remainingHostBits =
        hostBits - borrowedBits;

    /* ==============================
       SUBNET SIZE
       ============================== */

    int subnetSize = 1;

    for (int i = 0;
         i < remainingHostBits;
         i++)
    {
        subnetSize = subnetSize * 2;
    }

    int usableHosts =
        subnetSize - 2;

    /* ==============================
       SUBNET MASK
       ============================== */

    int mask[4];

    if (newPrefix <= 8)
    {
        mask[0] =
            256 - (1 << (8 - newPrefix));

        mask[1] = 0;
        mask[2] = 0;
        mask[3] = 0;
    }

    else if (newPrefix <= 16)
    {
        mask[0] = 255;

        int bits =
            newPrefix - 8;

        mask[1] =
            256 - (1 << (8 - bits));

        mask[2] = 0;
        mask[3] = 0;
    }

    else if (newPrefix <= 24)
    {
        mask[0] = 255;
        mask[1] = 255;

        int bits =
            newPrefix - 16;

        mask[2] =
            256 - (1 << (8 - bits));

        mask[3] = 0;
    }

    else
    {
        mask[0] = 255;
        mask[1] = 255;
        mask[2] = 255;

        int bits =
            newPrefix - 24;

        mask[3] =
            256 - (1 << (8 - bits));
    }

    /* ==============================
       BUILD RESULT HEADER
       ============================== */

    sprintf(result,

            "\n===============================================\n"
            "              SUBNETTING RESULT\n"
            "===============================================\n"
            "IP Address          : %s\n"
            "Class               : %c\n"
            "Number of Subnets   : %d\n"
            "Borrowed Bits       : %d\n"
            "New Prefix          : /%d\n"
            "Subnet Mask         : %d.%d.%d.%d\n"
            "Subnet Size         : %d addresses\n"
            "Usable Hosts        : %d\n"
            "===============================================\n"
            "%-10s %-22s %-22s\n"
            "===============================================\n",

            ip,
            className,
            totalSubnets,
            borrowedBits,
            newPrefix,
            mask[0],
            mask[1],
            mask[2],
            mask[3],
            subnetSize,
            usableHosts,
            "Subnet",
            "Subnet ID",
            "Broadcast ID");

    /* ==============================
       GENERATE SUBNETS
       ============================== */

    for (int i = 0;
         i < totalSubnets;
         i++)
    {
        int networkA = a;
        int networkB = b;
        int networkC = c;
        int networkD = d;

        int broadcastA = 0;
        int broadcastB = 0;
        int broadcastC = 0;
        int broadcastD = 0;

        /*
         * CLASS A
         */

        if (className == 'A')
        {
            int subnetNumber =
                i * subnetSize;

            networkB =
                (subnetNumber >> 16) & 255;

            networkC =
                (subnetNumber >> 8) & 255;

            networkD =
                subnetNumber & 255;

            int broadcastNumber =
                subnetNumber +
                subnetSize - 1;

            broadcastA = a;

            broadcastB =
                (broadcastNumber >> 16) & 255;

            broadcastC =
                (broadcastNumber >> 8) & 255;

            broadcastD =
                broadcastNumber & 255;
        }

        /*
         * CLASS B
         */

        else if (className == 'B')
        {
            int subnetNumber =
                i * subnetSize;

            networkC =
                (subnetNumber >> 8) & 255;

            networkD =
                subnetNumber & 255;

            int broadcastNumber =
                subnetNumber +
                subnetSize - 1;

            broadcastA = a;
            broadcastB = b;

            broadcastC =
                (broadcastNumber >> 8) & 255;

            broadcastD =
                broadcastNumber & 255;
        }

        /*
         * CLASS C
         */

        else
        {
            int subnetNumber =
                i * subnetSize;

            networkD =
                subnetNumber & 255;

            int broadcastNumber =
                subnetNumber +
                subnetSize - 1;

            broadcastA = a;
            broadcastB = b;
            broadcastC = c;

            broadcastD =
                broadcastNumber & 255;
        }

        /* ==============================
           CREATE IP STRINGS
           ============================== */

        char subnetID[50];
        char broadcastID[50];
        char line[200];

        sprintf(subnetID,
                "%d.%d.%d.%d",
                networkA,
                networkB,
                networkC,
                networkD);

        sprintf(broadcastID,
                "%d.%d.%d.%d",
                broadcastA,
                broadcastB,
                broadcastC,
                broadcastD);

        /* ==============================
           FORMAT TABLE ROW
           ============================== */

        sprintf(line,
                "%-10d %-22s %-22s\n",
                i + 1,
                subnetID,
                broadcastID);

        strcat(result, line);
    }

    strcat(result,
           "===============================================\n");

    /* ==============================
       SEND RESULT
       ============================== */

    send(client_fd,
         result,
         sizeof(result),
         0);

    printf("\nResult sent to client.\n");

    close(client_fd);
    close(server_fd);

    return 0;
}