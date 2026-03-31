#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 5000
#define MAX_SUBSCRIPTIONS 100
#define BUFFER_SIZE 1024
#define TOPIC_SIZE 64

struct subscription {
    struct sockaddr_in addr;
    char topic[TOPIC_SIZE];
};

struct subscription subs[MAX_SUBSCRIPTIONS];
int sub_count = 0;

int misma_direccion(struct sockaddr_in a, struct sockaddr_in b) {
    return (a.sin_addr.s_addr == b.sin_addr.s_addr) &&
           (a.sin_port == b.sin_port);
}

int suscripcion_existe(struct sockaddr_in addr, const char *topic) {
    for (int i = 0; i < sub_count; i++) {
        if (misma_direccion(subs[i].addr, addr) &&
            strcmp(subs[i].topic, topic) == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

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

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    printf("[BROKER UDP] Escuchando en puerto %d...\n", PORT);
    printf("[BROKER UDP] Formatos esperados:\n");
    printf("  SUBSCRIBE <tema>\n");
    printf("  MSG <tema> <contenido>\n\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr*)&client_addr, &addr_len);

        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';
        printf("[BROKER] Recibido desde %s:%d -> %s\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               buffer);

        if (strncmp(buffer, "SUBSCRIBE ", 10) == 0) {
            char topic[TOPIC_SIZE];

            if (sscanf(buffer, "SUBSCRIBE %63s", topic) == 1) {
                if (sub_count >= MAX_SUBSCRIPTIONS) {
                    printf("[BROKER] Maximo de suscripciones alcanzado\n");
                    continue;
                }

                if (!suscripcion_existe(client_addr, topic)) {
                    subs[sub_count].addr = client_addr;
                    strncpy(subs[sub_count].topic, topic, TOPIC_SIZE - 1);
                    subs[sub_count].topic[TOPIC_SIZE - 1] = '\0';
                    sub_count++;

                    printf("[BROKER] Suscriptor agregado al tema '%s' (%d suscripciones)\n",
                           topic, sub_count);
                } else {
                    printf("[BROKER] La suscripcion ya existia para tema '%s'\n", topic);
                }
            } else {
                printf("[BROKER] Mensaje SUBSCRIBE invalido\n");
            }

            continue;
        }

        if (strncmp(buffer, "MSG ", 4) == 0) {
            char topic[TOPIC_SIZE];
            char content[BUFFER_SIZE];

            if (sscanf(buffer, "MSG %63s %[^\n]", topic, content) == 2) {
                int enviados = 0;

                for (int i = 0; i < sub_count; i++) {
                    if (strcmp(subs[i].topic, topic) == 0) {
                        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                                          (struct sockaddr*)&subs[i].addr,
                                          sizeof(subs[i].addr));
                        if (sent < 0) {
                            perror("sendto");
                        } else {
                            enviados++;
                        }
                    }
                }

                printf("[BROKER] Reenviado a %d subscriber(s) del tema '%s'\n",
                       enviados, topic);
            } else {
                printf("[BROKER] Mensaje MSG invalido\n");
            }

            continue;
        }

        printf("[BROKER] Mensaje desconocido. Ignorado.\n");
    }

    close(sockfd);
    return 0;
}