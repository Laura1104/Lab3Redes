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

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, local_addr;
    char buffer[BUFFER_SIZE];

    if (argc < 2)
    {
        printf("Uso: %s <tema1> [tema2] [tema3] ...\n", argv[0]);
        printf("Ejemplo: %s PartidoA PartidoB\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }

    printf("[SUBSCRIBER UDP] Suscribiendose a:\n");

    for (int i = 1; i < argc; i++)
    {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "SUBSCRIBE %s", argv[i]);

        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));

        if (sent < 0)
        {
            perror("sendto");
        }
        else
        {
            printf("  - %s\n", argv[i]);
        }
    }

    printf("\n[SUBSCRIBER UDP] Esperando actualizaciones...\n\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);

        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';

        char topic[TOPIC_SIZE];
        char content[BUFFER_SIZE];

        if (sscanf(buffer, "MSG %63s %[^\n]", topic, content) == 2)
        {
            printf("[%s] %s\n", topic, content);
        }
        else
        {
            printf("[SUBSCRIBER] Mensaje recibido: %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}