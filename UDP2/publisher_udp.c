#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 5000
#define BUFFER_SIZE 1024
#define TOPIC_SIZE 64

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if (argc < 2) {
        printf("Uso: %s <tema>\n", argv[0]);
        printf("Ejemplo: %s PartidoA\n", argv[0]);
        return 1;
    }

    char *topic = argv[1];

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
        "Fin del partido"
    };

    int total_eventos = sizeof(eventos) / sizeof(eventos[0]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }

    printf("[PUBLISHER UDP] Tema: %s\n", topic);
    printf("[PUBLISHER UDP] Enviando eventos predefinidos...\n\n");

    for (int i = 0; i < total_eventos; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "MSG %s %s", topic, eventos[i]);

        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr*)&server_addr,
                          sizeof(server_addr));

        if (sent < 0) {
            perror("sendto");
            break;
        }

        printf("[PUBLISHER] Enviado: %s\n", buffer);
        sleep(1);
    }

    printf("\n[PUBLISHER UDP] Ahora puedes escribir mensajes manuales.\n");
    printf("Escribe texto y se enviara como: MSG %s <texto>\n", topic);
    printf("Ctrl + D para salir.\n\n");

    while (1) {
        char input[BUFFER_SIZE - 100];

        memset(input, 0, sizeof(input));

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "MSG %s %s", topic, input);

        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr*)&server_addr,
                          sizeof(server_addr));

        if (sent < 0) {
            perror("sendto");
            break;
        }

        printf("[PUBLISHER] Enviado: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}