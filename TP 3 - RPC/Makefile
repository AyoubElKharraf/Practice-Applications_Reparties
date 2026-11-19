CC = gcc
CFLAGS = -Wall -Wextra -g -I/usr/include/tirpc
LDLIBS = -ltirpc

RPC_SRC = calcule.x
CLIENT = client_calcule
SERVER = serveur

all: $(SERVER) $(CLIENT)

# Génération des stubs
calcule.h calcule_xdr.c calcule_clnt.c calcule_svc.c: $(RPC_SRC)
	rpcgen $(RPC_SRC)

# Objets stubs
calcule_xdr.o: calcule_xdr.c calcule.h
	$(CC) $(CFLAGS) -c calcule_xdr.c

calcule_clnt.o: calcule_clnt.c calcule.h
	$(CC) $(CFLAGS) -c calcule_clnt.c

calcule_svc.o: calcule_svc.c calcule.h
	$(CC) $(CFLAGS) -c calcule_svc.c

# Client
client_calcule.o: client_calcule.c calcule.h
	$(CC) $(CFLAGS) -c client_calcule.c

$(CLIENT): client_calcule.o calcule_clnt.o calcule_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Serveur
calcule_server.o: calcule_server.c calcule.h
	$(CC) $(CFLAGS) -c calcule_server.c

$(SERVER): calcule_svc.o calcule_server.o calcule_xdr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o $(CLIENT) $(SERVER)

clean-all: clean
	rm -f calcule.h calcule_xdr.c calcule_clnt.c calcule_svc.c
