/* Exercice 1 - Serveur TCP pour communication à distance (deux machines distinctes)
 * Usage : ./serveur_distant [port] (par défaut : 6000)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

#define PORT_DEFAUT 6000
#define TAILLEBUF 1024

/* Affiche les adresses IP réelles de la machine pour faciliter la connexion du client distant */
static void afficher_adresses_ip(int port) {
    struct ifaddrs *ifaddr, *ifa;
    printf("=================================================================\n");
    printf(" Adresse(s) IP disponibles pour joindre ce serveur à distance :\n");
    printf("-----------------------------------------------------------------\n");
    int trouve = 0;
    if (getifaddrs(&ifaddr) != -1) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)ifa->ifa_addr;
                char *ip = inet_ntoa(p->sin_addr);
                if (strcmp(ip, "127.0.0.1") != 0) {
                    printf("  -> Interface %-8s : %s (port %d)\n", ifa->ifa_name, ip, port);
                    printf("     Lancer sur le client : ./client_distant %s %d\n", ip, port);
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

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : PORT_DEFAUT;
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Erreur : port invalide '%s' (doit être compris entre 1 et 65535)\n", argv[1]);
        exit(1);
    }

    struct sockaddr_in addr_serveur, addr_client;
    socklen_t lg_addr = sizeof(addr_client);
    int s_ecoute, s_service, nb;
    char buf[TAILLEBUF];
    const char *reponse = "Message bien reçu par le serveur distant !";

    /* 1. Création de la socket d'écoute (IPv4, TCP) */
    s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ecoute == -1) {
        perror("socket");
        exit(1);
    }

    /* Option SO_REUSEADDR pour relancer immédiatement le serveur sans attendre TIME_WAIT */
    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. Nommage : INADDR_ANY écoute sur TOUTES les interfaces (WiFi, Ethernet, etc.) */
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

    printf("[SERVEUR DISTANT] En écoute sur le port %d (toutes interfaces)\n", port);
    afficher_adresses_ip(port);
    printf("En attente de connexion depuis une machine distante...\n");

    /* 4. Boucle pour accepter plusieurs clients consécutivement */
    while (1) {
        lg_addr = sizeof(addr_client);
        s_service = accept(s_ecoute, (struct sockaddr *)&addr_client, &lg_addr);
        if (s_service == -1) {
            perror("accept");
            continue;
        }

        /* Affichage de l'adresse IP et du port de la machine cliente distante */
        printf("\n>>> Client distant connecté : IP=%s, Port=%d\n",
               inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));

        /* 5. Réception du message */
        nb = recv(s_service, buf, TAILLEBUF - 1, 0);
        if (nb > 0) {
            buf[nb] = '\0';
            printf("Message reçu : \"%s\"\n", buf);

            /* 6. Envoi de la confirmation */
            if (send(s_service, reponse, strlen(reponse) + 1, 0) == -1) {
                perror("send");
            } else {
                printf("Réponse envoyée au client distant : \"%s\"\n", reponse);
            }
        } else if (nb == 0) {
            printf("Le client s'est déconnecté sans envoyer de données.\n");
        } else {
            perror("recv");
        }

        close(s_service);
        printf("Fin de connexion avec ce client. Prêt pour le suivant...\n");
    }

    close(s_ecoute);
    return 0;
}
