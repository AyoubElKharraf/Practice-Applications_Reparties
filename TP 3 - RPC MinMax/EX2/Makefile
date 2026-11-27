CC = gcc
RPCGEN = rpcgen
CFLAGS_GEN = -O2 -I/usr/include/tirpc
CFLAGS_USR = -Wall -O2 -I/usr/include/tirpc
LDFLAGS = -ltirpc -pthread

IDL = mat_mul.x
GEN_SRCS = mat_mul_clnt.c mat_mul_svc.c mat_mul_xdr.c
GEN_HDRS = mat_mul.h
GEN_OBJS = mat_mul_clnt.o mat_mul_svc.o mat_mul_xdr.o
USR_OBJS = mat_mul_server.o mat_mul_client.o

SERVER_BIN = serveur_mat
CLIENT_BIN = client_mat

.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN)

$(GEN_SRCS) $(GEN_HDRS): $(IDL)
	$(RPCGEN) -C $(IDL)

$(GEN_OBJS): %.o: %.c $(GEN_HDRS)
	$(CC) $(CFLAGS_GEN) -c $< -o $@

$(USR_OBJS): %.o: %.c $(GEN_HDRS)
	$(CC) $(CFLAGS_USR) -c $< -o $@

$(SERVER_BIN): mat_mul_svc.o mat_mul_xdr.o mat_mul_server.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(CLIENT_BIN): mat_mul_clnt.o mat_mul_xdr.o mat_mul_client.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(GEN_SRCS) $(GEN_HDRS) $(GEN_OBJS) $(USR_OBJS) $(SERVER_BIN) $(CLIENT_BIN)