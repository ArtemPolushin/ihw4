#include <random>
#include <vector>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <poll.h>
#include <string>
#include <sstream>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define M 2 // Количество рядов
#define N 2 // Количество шкафов в ряду
#define K 3 // Количество книг в шкафе
#define BUFSIZE 2048

struct Catalog {
    int data[M][N][K];
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Usage:  %s <Server port>\n", argv[0]);
        exit(1);
    }
    std::vector<int> v(M * N * K);
    std::iota(v.begin(), v.end(), 0);
    Catalog catalog;
    for (int m = 0; m < M; ++m) {
        for (int n = 0; n < N; ++n) {
            for (int k = 0; k < K; ++k) {
                std::cout << "Книга " << v[m * N * K + n * K + k] << " находится в " << m + 1 << " ряду, " << n + 1 << " шкафу, на " << k + 1 << " месте\n";
            }
        }
    }

    unsigned short servPort = atoi(argv[1]);
    int sock;
    struct sockaddr_in servAddr;
    struct sockaddr_in clntAddr;
    unsigned int clntLen;
    unsigned int cliAddrLen = sizeof(clntAddr);
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);
    if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }
    int row = 0;
    clntLen = sizeof(clntAddr);
    char buffer[BUFSIZE];
    if (recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen) < 0) {
        perror("recvfrom() failed");
        exit(1);
    } else {
        std::cout << buffer;
    }
    while (true)
    {
        const char* mes = std::to_string(row).c_str();
        int cnt;
        if ((cnt = sendto(sock, mes, strlen(mes), 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr))) != std::to_string(row).size()) {
            perror("send() failed");
            exit(1);
        }
        std::cout << "send client to deal with row #" << row << '\n';
        if ((cnt = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *) &clntAddr, &cliAddrLen)) < 0) {
            perror("recvfrom() failed");
            exit(1);
        }
        std::string mmes(buffer), mm;
        std::istringstream input2;
        input2.str(buffer);
        for (int n = 0; n < N; ++n) {
            for (int k = 0; k < K; ++k) {
                std::string line;
                std::getline(input2, line, ' ');
                int num = atoi(line.c_str());
                catalog.data[row][n][k] = num;
            }
        }
        if (++row == M) {
            close(sock); 
            break;
        }
    }
    for (int m = 0; m < M; ++m) {
        for (int n = 0; n < N; ++n) {
            for (int k = 0; k < K; ++k) {
                std::cout << "Книга " << catalog.data[m][n][k] << " находится в " << m + 1 << " ряду, " << n + 1 << " шкафу, на " << k + 1 << " месте\n";
            }
        }
    }
    return 0;
}