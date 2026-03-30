
#include <stdio.h>       
#include <stdlib.h>      
#include <string.h>      
#include <unistd.h>      
#include <arpa/inet.h>   
#include <sys/socket.h>  
#include <sys/select.h>  

#define PUERTO_BROKER   9000
#define MAX_CLIENTES    50
#define TAM_BUFFER      512
#define TAM_TEMA        64

/* Primero represento cada conexión porque el broker necesita memoria de quién es quién*/
typedef struct {
    int  id_socket;                 // Identificador del socket de ese cliente 
    char rol[16];                   // Saber el rol, quién me habla, si publicador o suscriptor
    char tema[TAM_TEMA];            // Partido al que está suscrito / que publica
} Cliente;

Cliente lista_clientes[MAX_CLIENTES];  //Base de datos de clientes
int total_clientes = 0;

/*Agregar cliente a la base de datos, si alguien nuevo se conecta*/
void agregar_cliente(int id_socket) { // Lo llamo cuando alguien se conecta y hay un nuevo id 

    if (total_clientes < MAX_CLIENTES) {

        lista_clientes[total_clientes].id_socket = id_socket;//guardo el socket 

        // Limpiar memoria (evita valores basura)
        memset(lista_clientes[total_clientes].rol, 0, sizeof(lista_clientes[total_clientes].rol));
        memset(lista_clientes[total_clientes].tema, 0, sizeof(lista_clientes[total_clientes].tema));

        total_clientes++;//aumento num de clientes
    }
}

/*Elimino cliente a la base de datos, si alguien se desconecta */
void eliminar_cliente(int id_socket) {

    for (int i = 0; i < total_clientes; i++) {

        if (lista_clientes[i].id_socket == id_socket) {
            close(id_socket); // cerrar conexión

            // Reemplazar con el último
            lista_clientes[i] = lista_clientes[total_clientes - 1];

            total_clientes--; //quito un cliente
            return;
        }
    }
}

/* Busco el cliente por el socket*/
int buscar_cliente(int id_socket) {

    for (int i = 0; i < total_clientes; i++) {
        if (lista_clientes[i].id_socket == id_socket)
            return i;
    }
    return -1;
}

int main(void) {

    int socket_servidor;              // Socket del broker
    struct sockaddr_in direccion;     // Dirección del broker
    fd_set sockets_lectura;           // Conjunto de sockets
    int max_socket;
    char mensaje[TAM_BUFFER];

    /* 1. Crear socket TCP */
    socket_servidor = socket(AF_INET, SOCK_STREAM, 0); //Aca se crea servidor TCP
    if (socket_servidor < 0) { perror("socket"); exit(1); }

    /* Permitir reutilizar puerto */
    int opcion = 1;
    setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion));

    /* 2. Bind */
    memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PUERTO_BROKER);

    if (bind(socket_servidor, (struct sockaddr*)&direccion, sizeof(direccion)) < 0) {
        perror("bind"); exit(1);
    }

    /* 3. Listen */
    if (listen(socket_servidor, 10) < 0) {
        perror("listen"); exit(1);
    }

    printf("[BROKER] Escuchando en puerto %d...\n", PUERTO_BROKER);

    /* Bucle*/
    while (1) {//Porque nunca se apaga

        FD_ZERO(&sockets_lectura); // Limpia el conjunto
        FD_SET(socket_servidor, &sockets_lectura);// Agrega el socket servidor
        max_socket = socket_servidor;

        /* Agregar clientes al select */
        for (int i = 0; i < total_clientes; i++) {

            FD_SET(lista_clientes[i].id_socket, &sockets_lectura);

            if (lista_clientes[i].id_socket > max_socket)
                max_socket = lista_clientes[i].id_socket;
        }

        /* Espera hasta que algún socket tenga actividad */
        int actividad = select(max_socket + 1, &sockets_lectura, NULL, NULL, NULL);

        if (actividad < 0) {
            perror("select");
            break;
        }

        /* NUEVA CONEXIÓN */
        if (FD_ISSET(socket_servidor, &sockets_lectura)) {

            struct sockaddr_in direccion_cliente;
            socklen_t tam = sizeof(direccion_cliente);

            int nuevo_socket = accept(socket_servidor, (struct sockaddr*)&direccion_cliente, &tam);

            if (nuevo_socket < 0) {
                perror("accept");
                continue;
            }

            agregar_cliente(nuevo_socket);

            printf("[BROKER] Nueva conexión desde %s (socket=%d)\n",
                   inet_ntoa(direccion_cliente.sin_addr), nuevo_socket);
        }


        /* MENSAJES DE CLIENTES */
        for (int i = 0; i < total_clientes; i++) {

            int id_socket = lista_clientes[i].id_socket;

            if (!FD_ISSET(id_socket, &sockets_lectura)) continue;

            memset(mensaje, 0, TAM_BUFFER);

            int bytes = read(id_socket, mensaje, TAM_BUFFER - 1);

            /* Cliente desconectado */
            if (bytes <= 0) {
                printf("[BROKER] Cliente socket=%d desconectado\n", id_socket);
                eliminar_cliente(id_socket);
                i--;
                continue;
            }

            mensaje[bytes] = '\0';

            /* REGISTRO */
            if (strncmp(mensaje, "REGISTER", 8) == 0) {

                char rol[16], tema[TAM_TEMA];

                sscanf(mensaje, "REGISTER %15s %63s", rol, tema);

                int pos = buscar_cliente(id_socket);

                if (pos >= 0) {
                    strncpy(lista_clientes[pos].rol, rol, sizeof(lista_clientes[pos].rol) - 1);
                    strncpy(lista_clientes[pos].tema, tema, sizeof(lista_clientes[pos].tema) - 1);
                }

                printf("[BROKER] Cliente %d es %s en '%s'\n", id_socket, rol, tema);
                continue;
            }

            /* MENSAJE */
            if (strncmp(mensaje, "MSG", 3) == 0) {

                char tema[TAM_TEMA];
                char contenido[TAM_BUFFER];

                // Separar 
                char *resto = mensaje + 4;
                char *espacio = strchr(resto, ' ');

                if (espacio != NULL) {

                    int largo_tema = espacio - resto;

                    strncpy(tema, resto, largo_tema);
                    tema[largo_tema] = '\0';

                    strcpy(contenido, espacio + 1);
                }

                printf("[BROKER] (%s) %s\n", tema, contenido);

                /* Reenviar a suscriptores */
                for (int j = 0; j < total_clientes; j++) {

                    if (strcmp(lista_clientes[j].rol, "subscriber") == 0 &&
                        strcmp(lista_clientes[j].tema, tema) == 0) {

                        write(lista_clientes[j].id_socket, mensaje, bytes);
                    }
                }
            }
        }
    }

    close(socket_servidor);
    return 0;
}