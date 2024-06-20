#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <string>

#define M 2 // Количество рядов
#define N 2 // Количество шкафов в ряду
#define K 3 // Количество книг в шкафе
#define BUFSIZE 2048

struct Catalog {
    int data[M][N][K];
};

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in servAddr;
    unsigned int fromSize;
    unsigned short servPort;
    char *servIP;

    if (argc != 3) {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
       exit(1);
    }

    servIP = argv[1];
    servPort = atoi(argv[2]);
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port        = htons(servPort);
    int row = 0;
    int cnt;
    std::string str = "Client ready to work\n";
    char buffer[BUFSIZE];
    if (sendto(sock, str.c_str(), str.length(), 0, (struct sockaddr *) &servAddr, sizeof(servAddr)) != str.length()) {
        perror("sendto() failed");
        exit(1);
    } else {
        std::cout << "Send first message\n";
    }
    while (row < M) {
        str.clear();
        cnt = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *) &servAddr, &fromSize);
        if (cnt < 0) {
            perror("recv() failed");
            exit(1);
        } else {
            row = atoi(buffer);
            printf("Dealing with row #%d\n", row);
            for (int n = 0; n < N; ++n) {
                for (int k = 0; k < K; ++k) {
                    str += std::to_string(row * N * K + n * K + k) + " ";
                }
            }
            str += "\0";
            if (sendto(sock, str.c_str(), str.length(), 0, (struct sockaddr *)
               &servAddr, sizeof(servAddr)) != str.length()) {
                perror("sendto() failed");
                break;
            }
            row++;
        }
        sleep(2);
    }
    close(sock);
    exit(0);
}
