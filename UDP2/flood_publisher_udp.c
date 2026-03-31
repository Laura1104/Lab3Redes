#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if (argc < 3) {
        printf("Uso: %s <tema> <cantidad_mensajes>\n", argv[0]);
        printf("Ejemplo: %s PartidoA 50000\n", argv[0]);
        return 1;
    }

    char *topic = argv[1];
    long total = atol(argv[2]);

    if (total <= 0) {
        printf("La cantidad de mensajes debe ser mayor que 0\n");
        return 1;
    }

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

    printf("[FLOOD UDP] Enviando %ld mensajes al tema '%s'...\n", total, topic);

    for (long i = 1; i <= total; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE,
                 "MSG %s FLOOD mensaje_numero_%ld prueba_saturacion_udp",
                 topic, i);

        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr*)&server_addr,
                          sizeof(server_addr));

        if (sent < 0) {
            perror("sendto");
            break;
        }

        if (i % 1000 == 0) {
            printf("[FLOOD UDP] %ld mensajes enviados\n", i);
        }
    }

    printf("[FLOOD UDP] Finalizado\n");

    close(sockfd);
    return 0;
}