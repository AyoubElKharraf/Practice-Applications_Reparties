CC     = gcc
CFLAGS = -Wall -Wextra

all: serveur_chat client_chat

serveur_chat: serveur_chat.c
	$(CC) $(CFLAGS) -o $@ $<

client_chat: client_chat.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f serveur_chat client_chat

.PHONY: all clean
