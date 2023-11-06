CC = gcc
CFLAGS = -Wall

all: chat_server chat_client

chat_server: chat_server.c
	$(CC) $(CFLAGS) -o chat_server chat_server.c

chat_client: chat_client.c
	$(CC) $(CFLAGS) -o chat_client chat_client.c
	
clean:
	rm -f server client
