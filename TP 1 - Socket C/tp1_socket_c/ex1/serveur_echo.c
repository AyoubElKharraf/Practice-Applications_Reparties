/* Exercice 1 - Serveur écho (majuscules), un client à la fois.
 * Compiler avec -DMULTI pour la version multi-clients (un fork par client). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 6000
#define TAILLEBUF 1024

/* dialogue avec UN client, jusqu'à "quit" ou fermeture de la connexion */
static void traiter_client(int s_service) {
    char buf[TAILLEBUF];
    int nb;

    while ((nb = recv(s_service, buf, TAILLEBUF - 1, 0)) > 0) {
        buf[nb] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';           /* retire le \n de fgets */

        if (strcmp(buf, "quit") == 0) {
            printf("[%d] le client a envoyé quit\n", getpid());
            fflush(stdout);
            break;
        }
        for (int i = 0; buf[i]; i++)
            buf[i] = toupper((unsigned char)buf[i]);
        send(s_service, buf, strlen(buf), 0);
    }
    /* nb == 0 : le client a fermé la connexion ; nb == -1 : erreur */
    close(s_service);
}

int main(void) {
    struct sockaddr_in addr_serveur, addr_client;
    socklen_t lg_addr;
    int s_ecoute, s_service;

#ifdef MULTI
    signal(SIGCHLD, SIG_IGN);   /* évite les processus zombies */
#endif

    s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ecoute == -1) { perror("socket"); exit(1); }
    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(PORT);
    addr_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s_ecoute, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("bind"); exit(1);
    }
    if (listen(s_ecoute, 5) == -1) { perror("listen"); exit(1); }
    printf("Serveur écho en écoute sur le port %d\n", PORT);

    while (1) {
        lg_addr = sizeof(addr_client);
        s_service = accept(s_ecoute, (struct sockaddr *)&addr_client, &lg_addr);
        if (s_service == -1) { perror("accept"); continue; }
        printf("Client connecté : %s:%d\n",
               inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));

#ifdef MULTI
        fflush(stdout);             /* vide le tampon AVANT fork, sinon le fils le duplique */
        if (fork() == 0) {          /* processus fils : s'occupe de ce client */
            close(s_ecoute);        /* le fils n'écoute pas */
            traiter_client(s_service);
            exit(0);
        }
        close(s_service);           /* le père n'a pas besoin de la socket de service */
#else
        traiter_client(s_service);  /* version séquentielle : un client à la fois */
#endif
    }
}
