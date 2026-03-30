#include <stdio.h>       // printf, perror
#include <stdlib.h>      // exit
#include <string.h>      // memset, snprintf
#include <unistd.h>      // close, write, sleep
#include <arpa/inet.h>   // sockaddr_in, htons, inet_pton
#include <sys/socket.h>  // socket, connect

#define IP_BROKER    "127.0.0.1"  // IP del broker (localhost)
#define PUERTO_BROKER  9000         // Puerto del broker
#define TAM_BUFFER  512

int main(int argc, char *argv[]) {

    /* 1. Verificar tema */

    if (argc < 2) {
        printf("Uso: %s <tema>\n", argv[0]);
        printf("Ejemplo: %s PartidoA\n", argv[0]);
        exit(1);
    }

    char *tema = argv[1]; // Partido que este publicador va a reportar


    /* Variables principales */

    int socket_cliente;                // Socket del publisher
    struct sockaddr_in direccion_broker; // Dirección del broker
    char mensaje[TAM_BUFFER];          // Buffer para enviar mensajes

    /* 2. Crear socket TCP */

    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_cliente < 0) {
        perror("socket");
        exit(1);
    }

    /* 3. Configurar dirección del broker */

    memset(&direccion_broker, 0, sizeof(direccion_broker));

    direccion_broker.sin_family = AF_INET;          // IPv4
    direccion_broker.sin_port   = htons(PUERTO_BROKER); // Puerto correcto

    // Convertir IP de texto a formato binario
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

    printf("[PUBLISHER] Conectado al broker. Tema: %s\n", tema);

    /* 5. Registrarse */

    // Primer mensaje SIEMPRE debe ser REGISTER
    // Le dices al broker quién eres y qué tema manejas

    snprintf(mensaje, TAM_BUFFER, "REGISTER publisher %s", tema);

    write(socket_cliente, mensaje, strlen(mensaje));

    printf("[PUBLISHER] Registrado como publisher en '%s'\n", tema);

    /* 6. Lista de eventos */

    const char *eventos[] = {
        "Inicio del partido",
        "Tarjeta amarilla al numero 5 de Equipo B",
        "Gol de Equipo A al minuto 23",
        "Cambio: jugador 10 entra por jugador 7",
        "Tiro al arco del Equipo B - rechazado",
        "Gol de Equipo B al minuto 41 - empate",
        "Fin del primer tiempo",
        "Inicio del segundo tiempo",
        "Gol de Equipo A al minuto 67 - ventaja",
        "Fin del partido - gana Equipo A 2-1"
    };

    int total_eventos = 10;

    /* 7. Enviar eventos uno por uno */

    for (int i = 0; i < total_eventos; i++) {

        memset(mensaje, 0, TAM_BUFFER);

        // Formato que espera el broker
        // "MSG <tema> <contenido>"
        snprintf(mensaje, TAM_BUFFER, "MSG %s %s", tema, eventos[i]);

        int enviados = write(socket_cliente, mensaje, strlen(mensaje));

        if (enviados < 0) {
            perror("write");
            break;
        }

        printf("[PUBLISHER] Enviado: %s\n", mensaje);

        // Simula tiempo real (1 segundo entre eventos)
        sleep(1);
    }


    printf("[PUBLISHER] Todos los mensajes enviados. Cerrando.\n");

    /* 8. Cerrar conexión */

    close(socket_cliente);

    return 0;
}