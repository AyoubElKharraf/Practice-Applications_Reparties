CC = gcc
RPCGEN = rpcgen
CFLAGS_GEN = -O2 -I/usr/include/tirpc
CFLAGS_USR = -Wall -O2 -I/usr/include/tirpc
LDFLAGS = -ltirpc -pthread

IDL = calc_tab.x
GEN_SRCS = calc_tab_clnt.c calc_tab_svc.c calc_tab_xdr.c
GEN_HDRS = calc_tab.h
GEN_OBJS = calc_tab_clnt.o calc_tab_svc.o calc_tab_xdr.o
USR_OBJS = calc_tab_server.o calc_tab_client.o

SERVER_BIN = serveur_tab
CLIENT_BIN = client_tab

.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN)

$(GEN_SRCS) $(GEN_HDRS): $(IDL)
	$(RPCGEN) -C $(IDL)

$(GEN_OBJS): %.o: %.c $(GEN_HDRS)
	$(CC) $(CFLAGS_GEN) -c $< -o $@

$(USR_OBJS): %.o: %.c $(GEN_HDRS)
	$(CC) $(CFLAGS_USR) -c $< -o $@

$(SERVER_BIN): calc_tab_svc.o calc_tab_xdr.o calc_tab_server.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(CLIENT_BIN): calc_tab_clnt.o calc_tab_xdr.o calc_tab_client.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(GEN_SRCS) $(GEN_HDRS) $(GEN_OBJS) $(USR_OBJS) $(SERVER_BIN) $(CLIENT_BIN)