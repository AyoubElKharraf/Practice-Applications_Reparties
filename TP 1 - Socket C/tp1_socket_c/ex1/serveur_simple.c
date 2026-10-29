/* Exercice 1 - Serveur TCP simple : un seul client, un seul message */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 6000
#define TAILLEBUF 256

int main(void) {
    struct sockaddr_in addr_serveur, addr_client;
    socklen_t lg_addr = sizeof(addr_client);
    int s_ecoute, s_service, nb;
    char buf[TAILLEBUF];
    const char *reponse = "Message bien reçu";

    /* 1. création de la socket d'écoute (TCP) */
    s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ecoute == -1) { perror("socket"); exit(1); }

    /* permet de relancer le serveur tout de suite (évite EADDRINUSE / TIME_WAIT) */
    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. nommage : adresse + port, en format réseau */
    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(PORT);
    addr_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s_ecoute, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("bind"); exit(1);
    }

    /* 3. file d'attente de 5 connexions pendantes */
    if (listen(s_ecoute, 5) == -1) { perror("listen"); exit(1); }
    printf("Serveur en écoute sur le port %d...\n", PORT);

    /* 4. accept : bloquant, retourne la socket de service */
    s_service = accept(s_ecoute, (struct sockaddr *)&addr_client, &lg_addr);
    if (s_service == -1) { perror("accept"); exit(1); }
    printf("Client connecté : %s:%d\n",
           inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));

    /* 5. réception puis réponse */
    nb = recv(s_service, buf, TAILLEBUF - 1, 0);
    if (nb == -1) { perror("recv"); exit(1); }
    buf[nb] = '\0';
    printf("Reçu : %s\n", buf);

    send(s_service, reponse, strlen(reponse) + 1, 0);

    close(s_service);
    close(s_ecoute);
    return 0;
}
