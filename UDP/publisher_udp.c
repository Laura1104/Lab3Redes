#include <stdio.h>      // Entrada/salida
#include <stdlib.h>     // Funciones generales
#include <string.h>     // Manejo de strings
#include <winsock2.h>   // Sockets en Windows

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;       // Inicialización de Winsock
    SOCKET sockfd;     // Socket
    struct sockaddr_in server_addr; // Dirección del broker
    char buffer[BUFFER_SIZE]; // Buffer para mensajes

    // Inicializar Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Configurar dirección del broker
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

    printf("Ingrese mensajes:\n");

    // Loop infinito para enviar mensajes
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin); // Leer desde teclado

        // Enviar mensaje al broker
        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr,
               sizeof(server_addr));
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}