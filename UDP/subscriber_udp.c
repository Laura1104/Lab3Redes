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

    // Inicializar Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Configurar dirección local (para recibir datos)
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0); // Puerto automático
    local_addr.sin_addr.s_addr = INADDR_ANY;

    // Asociar socket a la dirección local
    bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));

    // Configurar dirección del broker
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Enviar mensaje de suscripción
    strcpy(buffer, "SUBSCRIBE");

    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr*)&server_addr,
           sizeof(server_addr));

    printf("Suscrito al broker...\n");

    // Loop infinito para recibir mensajes
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Recibir mensajes del broker
        recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);

        printf("Mensaje recibido: %s\n", buffer);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}