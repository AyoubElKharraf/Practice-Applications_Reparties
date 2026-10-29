CC     = gcc
CFLAGS = -Wall -Wextra

all: ex1 ex2 versions

ex1:
	$(CC) $(CFLAGS) -o ex1/serveur_simple       ex1/serveur_simple.c
	$(CC) $(CFLAGS) -o ex1/client_simple        ex1/client_simple.c
	$(CC) $(CFLAGS) -o ex1/serveur_echo         ex1/serveur_echo.c
	$(CC) $(CFLAGS) -DMULTI -o ex1/serveur_echo_multi ex1/serveur_echo.c
	$(CC) $(CFLAGS) -o ex1/client_echo          ex1/client_echo.c
	$(CC) $(CFLAGS) -o ex1/serveur_distant      ex1/serveur_distant.c
	$(CC) $(CFLAGS) -o ex1/client_distant       ex1/client_distant.c
	$(CC) $(CFLAGS) -o ex1/serveur_echo_distant ex1/serveur_echo_distant.c
	$(CC) $(CFLAGS) -o ex1/client_echo_distant  ex1/client_echo_distant.c

ex2:
	$(CC) $(CFLAGS) -o ex2/serveur_ftp ex2/serveur_ftp.c
	$(CC) $(CFLAGS) -o ex2/client_ftp  ex2/client_ftp.c

versions:
	$(MAKE) -C ex2_versions

clean:
	rm -f ex1/serveur_simple ex1/client_simple ex1/serveur_echo ex1/serveur_echo_multi ex1/client_echo \
	      ex1/serveur_distant ex1/client_distant ex1/serveur_echo_distant ex1/client_echo_distant \
	      ex2/serveur_ftp ex2/client_ftp
	$(MAKE) -C ex2_versions clean

.PHONY: all ex1 ex2 versions clean
