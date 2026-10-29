/* Exercice 1 - Serveur écho TCP distant (multi-clients avec fork)
 * Usage : ./serveur_echo_distant [port] (par défaut : 6000)
 */
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
#include <ifaddrs.h>
#include <netdb.h>

#define PORT_DEFAUT 6000
#define TAILLEBUF 1024

/* Affiche les adresses IP réelles de la machine pour guider l'utilisateur */
/* Affiche les adresses IPv4 utilisables pour joindre le serveur depuis une autre machine. */
static void afficher_adresses_ip(int port) {
    struct ifaddrs *ifaddr, *ifa;
    printf("=================================================================\n");
    printf(" Adresse(s) IP pour connecter des clients distants :\n");
    printf("-----------------------------------------------------------------\n");
    int trouve = 0;
    if (getifaddrs(&ifaddr) != -1) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)ifa->ifa_addr;
                char *ip = inet_ntoa(p->sin_addr);
                if (strcmp(ip, "127.0.0.1") != 0) {
                    printf("  -> Interface %-8s : %s (port %d)\n", ifa->ifa_name, ip, port);
                    printf("     Sur le client : ./client_echo_distant %s %d\n", ip, port);
                    trouve = 1;
                }
            }
        }
        freeifaddrs(ifaddr);
    }
    if (!trouve) {
        printf("  (Seule la boucle locale a ete detectee dans cet environnement)\n");
    }
    printf("  -> En local (boucle)    : 127.0.0.1 (port %d)\n", port);
    printf("=================================================================\n\n");
}

/* Traitement d'un client distant dans son propre processus fils */
/* Gère une session d'écho avec un client distant et affiche son adresse. */
static void traiter_client(int s_service, struct sockaddr_in addr_client) {
    char buf[TAILLEBUF];
    int nb;
    char client_ip[INET_ADDRSTRLEN];
    int client_port = ntohs(addr_client.sin_port);
    inet_ntop(AF_INET, &addr_client.sin_addr, client_ip, sizeof(client_ip));

    printf("[Processus %d] Début de session avec le client distant %s:%d\n",
           getpid(), client_ip, client_port);
    fflush(stdout);

    while ((nb = recv(s_service, buf, TAILLEBUF - 1, 0)) > 0) {
        buf[nb] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        if (strcmp(buf, "quit") == 0) {
            printf("[Processus %d] Le client %s:%d a envoyé 'quit'. Fermeture.\n",
                   getpid(), client_ip, client_port);
            fflush(stdout);
            break;
        }

        printf("[Client %s:%d] Reçu : \"%s\"\n", client_ip, client_port, buf);

        /* Transformation en majuscules (écho) */
        for (int i = 0; buf[i]; i++) {
            buf[i] = toupper((unsigned char)buf[i]);
        }

        /* Renvoi de la phrase transformée */
        if (send(s_service, buf, strlen(buf), 0) == -1) {
            perror("send");
            break;
        }
    }

    if (nb == 0) {
        printf("[Processus %d] Le client distant %s:%d s'est déconnecté.\n",
               getpid(), client_ip, client_port);
    } else if (nb < 0) {
        perror("recv");
    }

    close(s_service);
    printf("[Processus %d] Session terminée.\n", getpid());
    fflush(stdout);
}
/* Point d'entrée : ouvre le serveur sur toutes les interfaces et distribue chaque connexion à un traitement client. */

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : PORT_DEFAUT;
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Erreur : port invalide '%s' (doit être compris entre 1 et 65535)\n", argv[1]);
        exit(1);
    }

    struct sockaddr_in addr_serveur, addr_client;
    socklen_t lg_addr;
    int s_ecoute, s_service;

    /* Évite la création de processus zombies pour les fils terminés */
    signal(SIGCHLD, SIG_IGN);

    /* 1. Création de la socket d'écoute */
    s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ecoute == -1) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. Nommage sur INADDR_ANY (écoute toutes les interfaces réseau) */
    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons((unsigned short)port);
    addr_serveur.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s_ecoute, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("bind");
        close(s_ecoute);
        exit(1);
    }

    /* 3. Mise en écoute */
    if (listen(s_ecoute, 5) == -1) {
        perror("listen");
        close(s_ecoute);
        exit(1);
    }

    printf("[SERVEUR ECHO DISTANT MULTI-CLIENTS] Actif sur le port %d\n", port);
    afficher_adresses_ip(port);

    while (1) {
        lg_addr = sizeof(addr_client);
        s_service = accept(s_ecoute, (struct sockaddr *)&addr_client, &lg_addr);
        if (s_service == -1) {
            perror("accept");
            continue;
        }

        fflush(stdout);
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(s_service);
            continue;
        }

        if (pid == 0) {
            /* Processus FILS : traite ce client */
            close(s_ecoute); /* Le fils n'écoute pas les nouvelles connexions */
            traiter_client(s_service, addr_client);
            exit(0);
        }

        /* Processus PÈRE : continue d'écouter */
        close(s_service);
    }

    close(s_ecoute);
    return 0;
}

