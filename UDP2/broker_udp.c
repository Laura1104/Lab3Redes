#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 5000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct subscriber {
    struct sockaddr_in addr;
};

struct subscriber subs[MAX_CLIENTS];
int sub_count = 0;

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("[BROKER UDP] Escuchando en puerto %d...\n", PORT);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr*)&client_addr, &addr_len);

        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';
        printf("[BROKER] Mensaje recibido: %s\n", buffer);

        // Registrar subscriber
        if (strncmp(buffer, "SUBSCRIBE", 9) == 0) {
            if (sub_count < MAX_CLIENTS) {
                subs[sub_count].addr = client_addr;
                sub_count++;
                printf("[BROKER] Subscriber registrado (%d)\n", sub_count);
            } else {
                printf("[BROKER] Maximo de subscribers alcanzado\n");
            }
            continue;
        }

        // Reenviar mensaje a todos
        for (int i = 0; i < sub_count; i++) {
            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr*)&subs[i].addr,
                   sizeof(subs[i].addr));
        }
    }

    close(sockfd);
    return 0;
}