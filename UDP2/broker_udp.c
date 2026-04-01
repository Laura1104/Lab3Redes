#include <stdio.h>      // Permite usar printf, perror, etc.
#include <stdlib.h>     // Permite usar exit()
#include <string.h>     // Permite usar memset, strcmp, strncpy, strncmp, sscanf
#include <arpa/inet.h>  // Permite trabajar con direcciones IP y funciones como htons, inet_ntoa
#include <sys/socket.h> // Define estructuras y funciones de sockets
#include <sys/types.h>  // Define tipos usados por sockets
#include <unistd.h>     // Permite usar close()

#define PORT 5000             // Puerto en el que el broker UDP va a escuchar
#define MAX_SUBSCRIPTIONS 100 // Número máximo de suscripciones permitidas
#define BUFFER_SIZE 1024      // Tamaño máximo del buffer para recibir mensajes
#define TOPIC_SIZE 64         // Tamaño máximo del nombre del tema

// Estructura que representa una suscripción:
// guarda la dirección del suscriptor y el tema al que está suscrito
struct subscription
{
    struct sockaddr_in addr; // Dirección IP y puerto del suscriptor
    char topic[TOPIC_SIZE];  // Tema al que está suscrito
};

struct subscription subs[MAX_SUBSCRIPTIONS]; // Arreglo que almacena todas las suscripciones
int sub_count = 0;                           // Contador actual de suscripciones registradas

// Función que compara dos direcciones de red para saber si son la misma
int misma_direccion(struct sockaddr_in a, struct sockaddr_in b)
{
    return (a.sin_addr.s_addr == b.sin_addr.s_addr) && // Compara IP
           (a.sin_port == b.sin_port);                 // Compara puerto
}

// Función que verifica si una suscripción ya existe
// Revisa si una misma dirección ya está suscrita al mismo tema
int suscripcion_existe(struct sockaddr_in addr, const char *topic)
{
    for (int i = 0; i < sub_count; i++)
    {                                              // Recorre todas las suscripciones guardadas
        if (misma_direccion(subs[i].addr, addr) && // Verifica si la dirección coincide
            strcmp(subs[i].topic, topic) == 0)
        {             // Verifica si el tema coincide
            return 1; // La suscripción ya existe
        }
    }
    return 0; // No existe esa suscripción
}

int main()
{
    int sockfd;                                  // Descriptor del socket UDP
    struct sockaddr_in server_addr, client_addr; // Estructuras para dirección del servidor y del cliente
    socklen_t addr_len = sizeof(client_addr);    // Tamaño de la estructura client_addr
    char buffer[BUFFER_SIZE];                    // Buffer para almacenar mensajes recibidos

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Crea un socket UDP (SOCK_DGRAM) usando IPv4 (AF_INET)
    if (sockfd < 0)
    {                     // Si hubo error al crear el socket
        perror("socket"); // Muestra el error
        exit(1);          // Termina el programa
    }

    memset(&server_addr, 0, sizeof(server_addr)); // Inicializa en cero la dirección del servidor
    memset(&client_addr, 0, sizeof(client_addr)); // Inicializa en cero la dirección del cliente

    server_addr.sin_family = AF_INET;         // Se usará IPv4
    server_addr.sin_port = htons(PORT);       // Se asigna el puerto 5000 en formato de red
    server_addr.sin_addr.s_addr = INADDR_ANY; // Acepta mensajes en cualquier interfaz de red

    // Asocia el socket a la dirección y puerto definidos
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind"); // Si falla el bind, muestra error
        close(sockfd);  // Cierra el socket
        exit(1);        // Termina el programa
    }

    // Mensajes informativos al iniciar el broker
    printf("[BROKER UDP] Escuchando en puerto %d...\n", PORT);
    printf("[BROKER UDP] Formatos esperados:\n");
    printf("  SUBSCRIBE <tema>\n");
    printf("  MSG <tema> <contenido>\n\n");

    while (1)
    {                                   // Bucle infinito: el broker siempre queda escuchando
        memset(buffer, 0, BUFFER_SIZE); // Limpia el buffer antes de recibir un nuevo mensaje

        // Espera a recibir un mensaje UDP
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&client_addr, &addr_len);

        if (n < 0)
        { // Si hubo error al recibir
            perror("recvfrom");
            continue; // Sigue esperando otro mensaje
        }

        buffer[n] = '\0'; // Agrega fin de cadena para tratar el mensaje como texto

        // Imprime quién envió el mensaje y su contenido
        printf("[BROKER] Recibido desde %s:%d -> %s\n",
               inet_ntoa(client_addr.sin_addr), // Convierte IP a texto
               ntohs(client_addr.sin_port),     // Convierte puerto a formato host
               buffer);                         // Mensaje recibido

        // Verifica si el mensaje comienza con "SUBSCRIBE "
        if (strncmp(buffer, "SUBSCRIBE ", 10) == 0)
        {
            char topic[TOPIC_SIZE]; // Variable para guardar el tema recibido

            // Extrae el tema del mensaje
            if (sscanf(buffer, "SUBSCRIBE %63s", topic) == 1)
            {
                if (sub_count >= MAX_SUBSCRIPTIONS)
                { // Verifica si ya no hay espacio
                    printf("[BROKER] Maximo de suscripciones alcanzado\n");
                    continue;
                }

                // Si la suscripción no existe aún, se agrega
                if (!suscripcion_existe(client_addr, topic))
                {
                    subs[sub_count].addr = client_addr;                    // Guarda la dirección del cliente
                    strncpy(subs[sub_count].topic, topic, TOPIC_SIZE - 1); // Copia el tema
                    subs[sub_count].topic[TOPIC_SIZE - 1] = '\0';          // Asegura fin de cadena
                    sub_count++;                                           // Incrementa el número total de suscripciones

                    printf("[BROKER] Suscriptor agregado al tema '%s' (%d suscripciones)\n",
                           topic, sub_count);
                }
                else
                {
                    // Si ya existía esa misma suscripción, lo informa
                    printf("[BROKER] La suscripcion ya existia para tema '%s'\n", topic);
                }
            }
            else
            {
                // Si el formato del SUBSCRIBE es incorrecto
                printf("[BROKER] Mensaje SUBSCRIBE invalido\n");
            }

            continue; // Vuelve al inicio del while para esperar otro mensaje
        }

        // Verifica si el mensaje comienza con "MSG "
        if (strncmp(buffer, "MSG ", 4) == 0)
        {
            char topic[TOPIC_SIZE];    // Variable para guardar el tema
            char content[BUFFER_SIZE]; // Variable para guardar el contenido del mensaje

            // Extrae el tema y el contenido del mensaje
            if (sscanf(buffer, "MSG %63s %[^\n]", topic, content) == 2)
            {
                int enviados = 0; // Contador de cuántos suscriptores recibieron el mensaje

                // Recorre todas las suscripciones registradas
                for (int i = 0; i < sub_count; i++)
                {
                    // Si la suscripción coincide con el tema del mensaje
                    if (strcmp(subs[i].topic, topic) == 0)
                    {
                        // Reenvía el mensaje completo a ese suscriptor
                        int sent = sendto(sockfd, buffer, strlen(buffer), 0,
                                          (struct sockaddr *)&subs[i].addr,
                                          sizeof(subs[i].addr));
                        if (sent < 0)
                        {
                            perror("sendto"); // Si falla el envío, muestra error
                        }
                        else
                        {
                            enviados++; // Cuenta el envío exitoso
                        }
                    }
                }

                // Informa cuántos suscriptores recibieron el mensaje
                printf("[BROKER] Reenviado a %d subscriber(s) del tema '%s'\n",
                       enviados, topic);
            }
            else
            {
                // Si el formato del mensaje MSG es incorrecto
                printf("[BROKER] Mensaje MSG invalido\n");
            }

            continue; // Vuelve al inicio del while
        }

        // Si el mensaje no era SUBSCRIBE ni MSG
        printf("[BROKER] Mensaje desconocido. Ignorado.\n");
    }

    close(sockfd); // Cierra el socket al terminar el programa (aunque aquí nunca se alcanza)
    return 0;      // Finaliza correctamente
}