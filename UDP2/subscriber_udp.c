#include <stdio.h>      // Permite usar printf y perror
#include <stdlib.h>     // Permite usar exit
#include <string.h>     // Permite usar memset, strlen, snprintf, sscanf
#include <arpa/inet.h>  // Permite usar inet_pton y estructuras de direcciones IP
#include <sys/socket.h> // Contiene funciones y estructuras para sockets
#include <sys/types.h>  // Define tipos usados por sockets
#include <unistd.h>     // Permite usar close

#define PORT 5000        // Puerto donde escucha el broker
#define BUFFER_SIZE 1024 // Tamaño máximo del buffer de mensajes
#define TOPIC_SIZE 64    // Tamaño máximo del nombre del tema

int main(int argc, char *argv[])
{
    int sockfd;                                 // Descriptor del socket UDP
    struct sockaddr_in server_addr, local_addr; // Dirección del broker y dirección local del subscriber
    char buffer[BUFFER_SIZE];                   // Buffer para enviar y recibir mensajes

    // Verifica que el usuario pase al menos un tema al ejecutar el programa
    if (argc < 2)
    {
        printf("Uso: %s <tema1> [tema2] [tema3] ...\n", argv[0]);
        printf("Ejemplo: %s PartidoA PartidoB\n", argv[0]);
        return 1; // Termina si no se pasan temas
    }

    // Crea el socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket"); // Muestra error si falla la creación del socket
        exit(1);
    }

    // Inicializa en cero la estructura de dirección local
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;         // Usa IPv4
    local_addr.sin_port = htons(0);          // Puerto 0: el sistema asigna un puerto libre automáticamente
    local_addr.sin_addr.s_addr = INADDR_ANY; // Escucha en cualquier interfaz local

    // Asocia el socket a una dirección local
    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("bind"); // Error si no puede asociarse al puerto local
        close(sockfd);
        exit(1);
    }

    // Inicializa en cero la dirección del broker
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;   // Usa IPv4
    server_addr.sin_port = htons(PORT); // Puerto del broker

    // Convierte la IP 127.0.0.1 a formato binario y la guarda en server_addr
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton"); // Error si la IP es inválida
        close(sockfd);
        exit(1);
    }

    // Informa los temas a los que se va a suscribir
    printf("[SUBSCRIBER UDP] Suscribiendose a:\n");

    // Recorre todos los temas pasados como argumentos
    for (int i = 1; i < argc; i++)
    {
        memset(buffer, 0, BUFFER_SIZE); // Limpia el buffer

        // Construye el mensaje de suscripción
        snprintf(buffer, BUFFER_SIZE, "SUBSCRIBE %s", argv[i]);

        // Envía la solicitud de suscripción al broker
        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));

        if (sent < 0)
        {
            perror("sendto"); // Muestra error si falla el envío
        }
        else
        {
            printf("  - %s\n", argv[i]); // Muestra el tema suscrito exitosamente
        }
    }

    // Indica que ya terminó de suscribirse y ahora queda esperando mensajes
    printf("\n[SUBSCRIBER UDP] Esperando actualizaciones...\n\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE); // Limpia el buffer antes de recibir

        // Espera mensajes reenviados por el broker
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);

        if (n < 0)
        {
            perror("recvfrom"); // Error si falla la recepción
            continue;
        }

        buffer[n] = '\0'; // Agrega fin de cadena para tratarlo como texto

        char topic[TOPIC_SIZE];    // Guardará el tema del mensaje recibido
        char content[BUFFER_SIZE]; // Guardará el contenido del mensaje recibido

        // Intenta separar el mensaje en formato: MSG <tema> <contenido>
        if (sscanf(buffer, "MSG %63s %[^\n]", topic, content) == 2)
        {
            // Si el formato es correcto, imprime el contenido mostrando el tema
            printf("[%s] %s\n", topic, content);
        }
        else
        {
            // Si no coincide con el formato esperado, imprime el mensaje completo
            printf("[SUBSCRIBER] Mensaje recibido: %s\n", buffer);
        }
    }

    close(sockfd); // Cierra el socket al terminar (aunque aquí nunca se alcanza)
    return 0;      // Finaliza el programa
}