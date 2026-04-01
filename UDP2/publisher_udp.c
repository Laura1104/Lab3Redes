#include <stdio.h>      // Permite usar printf, perror, fgets
#include <stdlib.h>     // Permite usar exit
#include <string.h>     // Permite usar memset, strlen, snprintf, strcspn
#include <arpa/inet.h>  // Funciones para manejo de direcciones IP (inet_pton)
#include <sys/socket.h> // Funciones y estructuras de sockets
#include <sys/types.h>  // Tipos necesarios para sockets
#include <unistd.h>     // Permite usar close, sleep

#define PORT 5000        // Puerto donde está escuchando el broker
#define BUFFER_SIZE 1024 // Tamaño del buffer para mensajes
#define TOPIC_SIZE 64    // Tamaño máximo del tema (aunque aquí no se usa directamente)

int main(int argc, char *argv[])
{
    int sockfd;                     // Descriptor del socket UDP
    struct sockaddr_in server_addr; // Dirección del broker
    char buffer[BUFFER_SIZE];       // Buffer para construir mensajes

    // Verifica que el usuario haya pasado el tema como argumento
    if (argc < 2)
    {
        printf("Uso: %s <tema>\n", argv[0]);
        printf("Ejemplo: %s PartidoA\n", argv[0]);
        return 1;
    }

    char *topic = argv[1]; // Tema al que se enviarán los mensajes

    // Lista de eventos predefinidos (simulación de partido)
    const char *eventos[] = {
        "Inicio del partido",
        "Tarjeta amarilla al numero 10 de Equipo B",
        "Gol de Equipo A al minuto 32",
        "Cambio: jugador 10 entra por jugador 20",
        "Tiro libre a favor de Equipo B",
        "Gol de Equipo B al minuto 45",
        "Fin del primer tiempo",
        "Inicio del segundo tiempo",
        "Tarjeta roja para Equipo A",
        "Fin del partido"};

    // Calcula cuántos eventos hay en el arreglo
    int total_eventos = sizeof(eventos) / sizeof(eventos[0]);

    // Crea el socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket"); // Error si no se pudo crear
        exit(1);
    }

    // Inicializa la estructura del servidor en cero
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;   // IPv4
    server_addr.sin_port = htons(PORT); // Puerto en formato de red

    // Convierte la IP "127.0.0.1" a formato binario
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton"); // Error si la IP es inválida
        close(sockfd);
        exit(1);
    }

    // Mensajes informativos iniciales
    printf("[PUBLISHER UDP] Tema: %s\n", topic);
    printf("[PUBLISHER UDP] Enviando eventos predefinidos...\n\n");

    // Envía los eventos uno por uno (simulación en tiempo real)
    for (int i = 0; i < total_eventos; i++)
    {
        memset(buffer, 0, BUFFER_SIZE); // Limpia el buffer

        // Construye el mensaje en formato esperado por el broker
        snprintf(buffer, BUFFER_SIZE, "MSG %s %s", topic, eventos[i]);

        // Envía el mensaje al broker
        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));

        if (sent < 0)
        {
            perror("sendto"); // Error al enviar
            break;
        }

        // Muestra el mensaje enviado
        printf("[PUBLISHER] Enviado: %s\n", buffer);

        sleep(1); // Espera 1 segundo entre eventos (simula transmisión en vivo)
    }

    // Indica que ahora el usuario puede escribir mensajes manuales
    printf("\n[PUBLISHER UDP] Ahora puedes escribir mensajes manuales.\n");
    printf("Escribe texto y se enviara como: MSG %s <texto>\n", topic);
    printf("Ctrl + D para salir.\n\n");

    // Bucle para entrada manual desde teclado
    while (1)
    {
        char input[BUFFER_SIZE - 100]; // Buffer para lo que escribe el usuario

        memset(input, 0, sizeof(input)); // Limpia el input

        // Lee una línea desde stdin
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break; // Sale si el usuario presiona Ctrl+D
        }

        // Elimina el salto de línea '\n'
        input[strcspn(input, "\n")] = '\0';

        // Si la entrada está vacía, ignora
        if (strlen(input) == 0)
        {
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE); // Limpia buffer

        // Construye el mensaje con el input del usuario
        snprintf(buffer, BUFFER_SIZE, "MSG %s %s", topic, input);

        // Envía el mensaje al broker
        int sent = sendto(sockfd, buffer, strlen(buffer), 0, 
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));

        if (sent < 0)
        {
            perror("sendto");
            break;
        }

        // Muestra el mensaje enviado
        printf("[PUBLISHER] Enviado: %s\n", buffer);
    }

    close(sockfd); // Cierra el socket al finalizar
    return 0;      // Fin del programa
}