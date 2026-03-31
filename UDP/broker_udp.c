#include <stdio.h>      // Librería estándar para entrada/salida (printf)
#include <stdlib.h>     // Librería estándar (funciones generales)
#include <string.h>     // Para manejo de strings (strcmp, memset, etc.)
#include <winsock2.h>   // Librería de sockets en Windows

// Indica al compilador que use la librería de sockets de Windows
#pragma comment(lib, "ws2_32.lib")

#define PORT 5000            // Puerto donde el broker escuchará
#define MAX_CLIENTS 10       // Número máximo de suscriptores
#define BUFFER_SIZE 1024     // Tamaño máximo del mensaje

// Estructura para guardar la dirección de un subscriber
struct subscriber {
    struct sockaddr_in addr; // Dirección IP + puerto del cliente
};

// Arreglo donde se guardan los subscribers
struct subscriber subs[MAX_CLIENTS];

// Contador de subscribers registrados
int sub_count = 0;

int main() {
    WSADATA wsa;   // Estructura para inicializar Winsock
    SOCKET sockfd; // Descriptor del socket
    struct sockaddr_in server_addr, client_addr; // Direcciones
    int addr_len = sizeof(client_addr); // Tamaño de la dirección
    char buffer[BUFFER_SIZE]; // Buffer para recibir mensajes

    // Inicializar Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Crear socket UDP (SOCK_DGRAM)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Configuración del servidor
    server_addr.sin_family = AF_INET;        // IPv4
    server_addr.sin_port = htons(PORT);      // Puerto (network byte order)
    server_addr.sin_addr.s_addr = INADDR_ANY;// Acepta cualquier IP

    // Asociar socket con la dirección y puerto
    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Broker UDP escuchando en puerto %d...\n", PORT);

    // Loop infinito para recibir mensajes
    while (1) {
        memset(buffer, 0, BUFFER_SIZE); // Limpiar buffer

        // Recibir mensaje desde cualquier cliente
        recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                 (struct sockaddr*)&client_addr, &addr_len);

        printf("Mensaje recibido: %s\n", buffer);

        // Registro de suscriptores
        if (strncmp(buffer, "SUBSCRIBE", 9) == 0) {
            subs[sub_count++] = (struct subscriber){client_addr};
            printf("Subscriber registrado (%d)\n", sub_count);
            continue;
        }

        // Reenviar mensaje a todos los suscriptores registrados
        for (int i = 0; i < sub_count; i++) {
            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr*)&subs[i].addr,
                   sizeof(subs[i].addr));
        }
    }

     // Cerrar socket
    closesocket(sockfd);
    // Limpiar Winsock
    WSACleanup();
    return 0;
}