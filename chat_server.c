/*
    Uso de server:
    unicamente poner ./chat_server e ignorar

    PARA AÑADIR MAS CLIENTES CAMBIAR #define MAX_CLIENTS 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080

//CAMBIAR PARA AÑADIR MAS CLIENTES
#define MAX_CLIENTS 3

#define RESET_COLOR    "\x1b[0m"
#define NEGRO_T        "\x1b[30m"
#define NEGRO_F        "\x1b[40m"
#define ROJO_T     "\x1b[31m"
#define ROJO_F     "\x1b[41m"
#define VERDE_T        "\x1b[32m"
#define VERDE_F        "\x1b[42m"
#define AMARILLO_T "\x1b[33m"
#define    AMARILLO_F  "\x1b[43m"
#define AZUL_T     "\x1b[34m"
#define    AZUL_F      "\x1b[44m"
#define MAGENTA_T  "\x1b[35m"
#define MAGENTA_F  "\x1b[45m"
#define CYAN_T     "\x1b[36m"
#define CYAN_F     "\x1b[46m"
#define BLANCO_T   "\x1b[37m"
#define BLANCO_F   "\x1b[47m"

const char *colores[] = {ROJO_T, VERDE_T,AZUL_T, AMARILLO_T,
                              MAGENTA_T, CYAN_T, NEGRO_T, BLANCO_T};

struct message {
    int ids[MAX_CLIENTS];
    int i;
    char *name;
};

void *thread_client(void *thread) {
	char buffer[1000], buffer2[1024];
    int i , j;

	struct message *sockfd = ((struct message *)thread);
    while (1) {
        int bytesRead = recv(sockfd->ids[sockfd->i], buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            printf("Cliente desconectado thread.\n");
            close(sockfd->ids[sockfd->i]);
            break;
        }
        if (strlen(buffer) > 0) {
            sprintf(buffer2, "%s>%s: %s%s",colores[sockfd->i], sockfd->name, buffer,RESET_COLOR);
            for( i = 1; i < MAX_CLIENTS; i++) {   
                j = (sockfd->i + i)%MAX_CLIENTS;
                ssize_t bytesSent = send(sockfd->ids[j], buffer2, sizeof(buffer2), 0);

                if (bytesSent == -1) {
                    perror("Error al enviar datos");
                }
            }
        }
    }
    pthread_exit(NULL);
}


int main() {
    setbuf(stdout, NULL);
    int serverSocket, clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddr, clientAddr;
    char name[MAX_CLIENTS][1000];
    char buffer[1024];
    socklen_t addrSize = sizeof(struct sockaddr_in);
    pthread_t thread[MAX_CLIENTS];
    const int enable = 1;
    struct message *sockfd[MAX_CLIENTS];
    int g, i, j;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error al crear el socket del servidor");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (setsockopt
	    (serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
		close(serverSocket);
		exit(EXIT_FAILURE);
	}

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error al enlazar el socket del servidor");
        exit(1);
    }

    listen(serverSocket, MAX_CLIENTS);

    printf("Esperando conexiones de clientes...\n");
 
    for (g = 0; g < MAX_CLIENTS; g++){
        clientSockets[g] = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrSize);
        int bytesRead = recv(clientSockets[g], buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            printf("Cliente desconectado  recv.\n");
            close(clientSockets[g]);
        }

        int length = sizeof(buffer);
        for (i = 0, j = 0; i < length; i++) {
            if (buffer[i] == '\n') {
                name[g][j++] = '-';
                name[g][j++] = ' ';
            } else {
                name[g][j++] = buffer[i];
            }
        }
        name[g][j] = '\0';        
        
        const char *mensaje = " conectado perfectamente\n";
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (buffer[i] == '\n') {
                buffer[i] = ' ';
            }
        }
        
        strcat(buffer, mensaje);
        bytesRead = send(clientSockets[g], buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            printf("Cliente desconectado send.\n");
            close(clientSockets[g]);
        }
        memset(buffer,0,sizeof(buffer));
        fprintf(stderr,"cliente %d conectado\n", g);
    }
    for (g = 0; g < MAX_CLIENTS; g++){
        sockfd[g] = (struct message *)malloc(sizeof(struct message));
	    memcpy(sockfd[g]->ids, clientSockets, sizeof(clientSockets));
        sockfd[g]->i = g;
        sockfd[g]->name = name[g];
        pthread_create(&thread[g], NULL, (void *)thread_client, (void *)sockfd[g]);
    }    

    for (g = 0; g < MAX_CLIENTS; g++){
        pthread_join(thread[g], NULL);
    }    
    
    return 0;
}

