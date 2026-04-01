#include <stdio.h>      // Permite usar printf, perror
#include <stdlib.h>     // Permite usar exit, atol
#include <string.h>     // Permite usar memset, strlen, snprintf
#include <arpa/inet.h>  // Funciones para manejo de direcciones IP (inet_pton)
#include <sys/socket.h> // Funciones y estructuras de sockets
#include <sys/types.h>  // Tipos necesarios para sockets
#include <unistd.h>     // Permite usar close()

#define PORT 5000        // Puerto al que se enviarán los mensajes (broker)
#define BUFFER_SIZE 1024 // Tamaño máximo del buffer

int main(int argc, char *argv[])
{
    int sockfd;                     // Descriptor del socket
    struct sockaddr_in server_addr; // Dirección del servidor (broker)
    char buffer[BUFFER_SIZE];       // Buffer para construir los mensajes

    // Verifica que el usuario haya pasado suficientes argumentos
    if (argc < 3)
    {
        printf("Uso: %s <tema> <cantidad_mensajes>\n", argv[0]);
        printf("Ejemplo: %s PartidoA 50000\n", argv[0]);
        return 1; // Termina el programa si faltan argumentos
    }

    char *topic = argv[1];      // Tema al que se enviarán los mensajes
    long total = atol(argv[2]); // Convierte la cantidad de mensajes a número

    // Verifica que la cantidad de mensajes sea válida
    if (total <= 0)
    {
        printf("La cantidad de mensajes debe ser mayor que 0\n");
        return 1;
    }

    // Crea un socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket"); // Muestra error si falla
        exit(1);
    }

    // Inicializa en cero la estructura de dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;   // Usa IPv4
    server_addr.sin_port = htons(PORT); // Define el puerto en formato de red

    // Convierte la IP "127.0.0.1" a formato binario y la guarda
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton"); // Error si la IP no es válida
        close(sockfd);
        exit(1);
    }

    // Mensaje informativo de inicio
    printf("[FLOOD UDP] Enviando %ld mensajes al tema '%s'...\n", total, topic);

    // Bucle que envía todos los mensajes
    for (long i = 1; i <= total; i++)
    {
        memset(buffer, 0, BUFFER_SIZE); // Limpia el buffer

        // Construye el mensaje en formato esperado por el broker
        snprintf(buffer, BUFFER_SIZE,
                 "MSG %s FLOOD mensaje_numero_%ld prueba_saturacion_udp",
                 topic, i);

        // Envía el mensaje al broker usando UDP
        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));

        // Verifica si hubo error al enviar
        if (sent < 0)
        {
            perror("sendto");
            break; // Sale del ciclo si falla
        }

        // Cada 1000 mensajes, imprime progreso
        if (i % 1000 == 0)
        {
            printf("[FLOOD UDP] %ld mensajes enviados\n", i);
        }
    }

    // Mensaje final cuando termina el envío
    printf("[FLOOD UDP] Finalizado\n");

    close(sockfd); // Cierra el socket
    return 0;      // Termina el programa correctamente
}