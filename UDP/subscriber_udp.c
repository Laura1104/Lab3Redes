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
    struct sockaddr_in server_addr, local_addr;
    int addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

    WSAStartup(MAKEWORD(2, 2), &wsa);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind local
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Enviar suscripción
    strcpy(buffer, "SUBSCRIBE");
    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr*)&server_addr,
           sizeof(server_addr));

    printf("Suscrito al broker...\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

        printf("Mensaje recibido: %s\n", buffer);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}