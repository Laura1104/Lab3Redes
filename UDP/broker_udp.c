#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct subscriber {
    struct sockaddr_in addr;
};

struct subscriber subs[MAX_CLIENTS];
int sub_count = 0;

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Inicializar Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Broker UDP escuchando en puerto %d...\n", PORT);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr*)&client_addr, &addr_len);

        printf("Mensaje recibido: %s\n", buffer);

        // Registro de suscriptores
        if (strncmp(buffer, "SUBSCRIBE", 9) == 0) {
            subs[sub_count++] = (struct subscriber){client_addr};
            printf("Subscriber registrado (%d)\n", sub_count);
            continue;
        }

        // Reenviar mensaje a todos
        for (int i = 0; i < sub_count; i++) {
            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr*)&subs[i].addr,
                   sizeof(subs[i].addr));
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}