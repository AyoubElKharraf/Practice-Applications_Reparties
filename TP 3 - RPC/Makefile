# Makefile pour le TP RPC
CC = gcc
CFLAGS = -Wall -Wextra -g
# Sous les distributions Linux modernes, la bibliothèque libtirpc est requise pour RPC
# Si libtirpc est présent, on inclut ses flags automatiquement
RPC_CFLAGS = $(shell pkg-config --cflags libtirpc 2>/dev/null)
RPC_LIBS   = $(shell pkg-config --libs libtirpc 2>/dev/null)

CFLAGS += $(RPC_CFLAGS)
LDLIBS += $(RPC_LIBS)

# Fichiers sources et cibles
RPC_SRC = calcule.x
CLIENT = client_calcule
SERVER = serveur

all: $(SERVER) $(CLIENT)

# Génération automatique des stubs et headers via rpcgen
calcule.h calcule_xdr.c calcule_clnt.c calcule_svc.c: $(RPC_SRC)
	rpcgen $(RPC_SRC)

# Compilation des objets générés
calcule_xdr.o: calcule_xdr.c calcule.h
	$(CC) $(CFLAGS) -c calcule_xdr.c

calcule_clnt.o: calcule_clnt.c calcule.h
	$(CC) $(CFLAGS) -c calcule_clnt.c

calcule_svc.o: calcule_svc.c calcule.h
	$(CC) $(CFLAGS) -c calcule_svc.c

# Compilation du client
client_calcule.o: client_calcule.c calcule.h
	$(CC) $(CFLAGS) -c client_calcule.c

$(CLIENT): client_calcule.o calcule_clnt.o calcule_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Compilation du serveur
calcule_server.o: calcule_server.c calcule.h
	$(CC) $(CFLAGS) -c calcule_server.c

$(SERVER): calcule_svc.o calcule_server.o calcule_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o $(CLIENT) $(SERVER)

clean-all: clean
	rm -f calcule.h calcule_xdr.c calcule_clnt.c calcule_svc.c
