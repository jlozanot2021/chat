/*
    Uso de cliente:
	poner ./chat_client [Ip de server] [puerto (creo que solo va 8080)]
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>


#define MAX_BUFFER 1024
#define PORT 8080

int tcp_socket;

void finish(int fd) {
	close(tcp_socket);
	exit(EXIT_SUCCESS);
}
 
int start(int sockfd) {
	char buffer[MAX_BUFFER];

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "Hello server!");
	printf("%s\n", buffer);
	if (send(sockfd, buffer, sizeof(buffer), 0) == -1) {
		fprintf(stderr, "Error in send\n");
		return -1;
	}
	return 0;
}

void *thread_client(void *thread) {
	int nbytes = 0, *sockfd;
	char buffer[MAX_BUFFER];

	sockfd = ((int *)thread);

	while (1) {
		nbytes = recv(*sockfd, (void *)buffer, sizeof(buffer), 0);
		if (nbytes == -1) {
			fprintf(stderr, "Error in recv\n");
			close(*sockfd);
			free(sockfd);
			pthread_exit(NULL);
		}

		buffer[nbytes] = '\0';
		printf("%s", buffer);
	}
}

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	signal(SIGINT, finish);
	struct sockaddr_in my_addr;
	int nbytes = 0;
	char buffer[MAX_BUFFER];
	pthread_t thread;

	int opt, port;
	char *ip = NULL;

    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
			default: /* '?' */
			    fprintf(stderr, "Unrecognized option: %c\n", opt);
				fprintf(stderr, "Usage: %s [ip] [port]\n",
						argv[0]);
				exit(EXIT_FAILURE);
        }
    }
	
    if (1 + optind >= argc) {
        fprintf(stderr, "Usage: %s [ip] [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ip = argv[optind];
    port = atoi(argv[optind + 1]);
    
    my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr(ip);
	my_addr.sin_port = htons(port);
	
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		fprintf(stderr, "Error in socket creation\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Socket successfully created...\n");
	}

	if (connect(tcp_socket, (struct sockaddr *)&my_addr,
		    sizeof(my_addr)) == -1) {
		fprintf(stderr, "Error in connect\n");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	} else {
		printf("connected to the server...\n");
	}

    char *username = getenv("USER");  // Intenta obtener el nombre de usuario de la variable de entorno "USER"
    if (username == NULL) {
        username = getenv("LOGNAME");  // Intenta obtener el nombre de usuario de la variable de entorno "LOGNAME" (por si "USER" no está definida)
    }
	nbytes = send(tcp_socket, username, sizeof(username), 0);
	if (nbytes == -1) {
		fprintf(stderr, "Error in send\n");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	}

	/* Si quieres poner otros nombres usuarios

	if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        nbytes = send(tcp_socket, buffer, sizeof(buffer), 0);
        if (nbytes == -1) {
            fprintf(stderr, "Error in send\n");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }
    }*/

	int *sockfd = (int *)malloc(sizeof(int));
	*sockfd = tcp_socket;
	pthread_create(&thread, NULL, (void *)thread_client, (void *)sockfd);
	while (tcp_socket >= 0) {
		memset(buffer, 0, sizeof(buffer));
		if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
			nbytes = send(tcp_socket, buffer, sizeof(buffer), 0);
			if (nbytes == -1) {
				fprintf(stderr, "Error in send\n");
				close(tcp_socket);
				exit(EXIT_FAILURE);
			}
		}
	}
    close(tcp_socket);
	exit(EXIT_SUCCESS);
}
