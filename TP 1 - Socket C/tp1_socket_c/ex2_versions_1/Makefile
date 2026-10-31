CC     = gcc
CFLAGS = -Wall -Wextra

all: v1 v2

v1:
	$(CC) $(CFLAGS) -o serveur_v1 serveur_v1.c
	$(CC) $(CFLAGS) -o client_v1  client_v1.c

v2:
	$(CC) $(CFLAGS) -o serveur_v2 serveur_v2.c
	$(CC) $(CFLAGS) -o client_v2  client_v2.c

clean:
	rm -f serveur_v1 client_v1 serveur_v2 client_v2
	rm -rf racine_ftp

.PHONY: all v1 v2 clean
