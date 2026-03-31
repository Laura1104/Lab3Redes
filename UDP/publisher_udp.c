#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    WSAStartup(MAKEWORD(2, 2), &wsa);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Ingrese mensajes:\n");

    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr,
               sizeof(server_addr));
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}