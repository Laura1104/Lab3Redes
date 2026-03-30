#include <stdio.h>       
#include <stdlib.h>      
#include <string.h>      
#include <unistd.h>      
#include <arpa/inet.h>   
#include <sys/socket.h>  

#define IP_BROKER     "127.0.0.1"
#define PUERTO_BROKER 9000
#define TAM_BUFFER    512

int main(int argc, char *argv[]) {

    /* 1. Verificar tema */

    if (argc < 2) {
        printf("Uso: %s <tema>\n", argv[0]);
        printf("Ejemplo: %s PartidoA\n", argv[0]);
        exit(1);
    }

    char *tema = argv[1]; // Tema que quiero seguir


    /* Variables principales */

    int socket_cliente;                  // Socket del subscriber
    struct sockaddr_in direccion_broker; // Dirección del broker
    char mensaje[TAM_BUFFER];            // Buffer para recibir mensajes

    /* 2. Crear socket TCP */

    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_cliente < 0) {
        perror("socket");
        exit(1);
    }

    /* 3. Configurar dirección del broker */

    memset(&direccion_broker, 0, sizeof(direccion_broker));

    direccion_broker.sin_family = AF_INET;
    direccion_broker.sin_port   = htons(PUERTO_BROKER);

    if (inet_pton(AF_INET, IP_BROKER, &direccion_broker.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    /* 4. Conectarse al broker */

    if (connect(socket_cliente,
               (struct sockaddr*)&direccion_broker,
               sizeof(direccion_broker)) < 0) {

        perror("connect");
        printf("¿Está corriendo el broker?\n");
        exit(1);
    }

    printf("[SUBSCRIBER] Conectado. Siguiendo tema: %s\n", tema);

    /* 5. Registrarse como suscriptor */

    snprintf(mensaje, TAM_BUFFER, "REGISTER subscriber %s", tema);

    write(socket_cliente, mensaje, strlen(mensaje));

    printf("[SUBSCRIBER] Suscrito a '%s'. Esperando mensajes...\n\n", tema);

    /* 6. Bucle para recibir */

    while (1) {

        memset(mensaje, 0, TAM_BUFFER);

        int bytes = read(socket_cliente, mensaje, TAM_BUFFER - 1);

        /* Si el broker cierra conexión */
        if (bytes <= 0) {
            printf("\n[SUBSCRIBER] Conexión cerrada por el broker.\n");
            break;
        }

        mensaje[bytes] = '\0';

        /* Separar mensaje recibido */

        // Ejemplo recibido:
        // "MSG PartidoA Gol minuto 23"

        char tema_recibido[64];
        char contenido[TAM_BUFFER];

        if (sscanf(mensaje, "MSG %63s %[^\n]", tema_recibido, contenido) == 2) {

            printf(" [%s] %s\n", tema_recibido, contenido);

        } else {

            // Por si algo raro llega
            printf(" Mensaje recibido: %s\n", mensaje);
        }
    }

    /* 7. Cerrar conexión */

    close(socket_cliente);

    printf("[SUBSCRIBER] Desconectado.\n");

    return 0;
}