/* ==============================================================================
 * EXERCICE 2 - VERSION 1 : Serveur distant (Messages & Commandes)
 * Usage : ./serveur_v1 [port]
 * ============================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#define PORT_DEFAUT 5000
#define BUF_SIZE 1024

// Fonction : Affiche les IP réseau réelles pour guider la connexion cliente
/* Affiche les adresses IP locales utilisables par un client distant. */
static void afficher_ip(int port) {
    struct ifaddrs *ifaddr, *ifa;
    printf("[SERVEUR V1] En ecoute sur le port %d\n", port);
    printf("Adresses pour connecter le client distant :\n");
    if (getifaddrs(&ifaddr) != -1) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)ifa->ifa_addr;
                char *ip = inet_ntoa(p->sin_addr);
                if (strcmp(ip, "127.0.0.1") != 0)
                    printf("  -> IP : %s | Commande : ./client_v1 %s %d\n", ip, ip, port);
            }
        }
        freeifaddrs(ifaddr);
    }
    printf("  -> Local : 127.0.0.1 (./client_v1 127.0.0.1 %d)\n\n", port);
}

// Fonction : Gère le dialogue avec un client distant (processus fils)
/* Gère le dialogue d'un client V1 et répond aux commandes prévues par le protocole. */
static void traiter_client(int sock, struct sockaddr_in addr) {
    char buf[BUF_SIZE], rep[BUF_SIZE * 2];
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    int port = ntohs(addr.sin_port);

    printf("[%d] Client connecte : %s:%d\n", getpid(), ip, port);

    // Boucle de réception des messages
    while (1) {
        int nb = recv(sock, buf, sizeof(buf) - 1, 0);
        if (nb <= 0) break; // Client déconnecté ou erreur

        buf[nb] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0'; // Retire saut de ligne

        printf("[%d] Recu de %s : \"%s\"\n", getpid(), ip, buf);

        // Traitement des commandes simples
        if (!strcasecmp(buf, "quit")) {
            snprintf(rep, sizeof(rep), "Deconnexion confirmee.\n");
            send(sock, rep, strlen(rep), 0);
            break;
        } else if (!strcasecmp(buf, "ping")) {
            snprintf(rep, sizeof(rep), "PONG (serveur joignable)\n");
        } else if (!strcasecmp(buf, "date")) {
            time_t t = time(NULL);
            snprintf(rep, sizeof(rep), "Date serveur : %s", ctime(&t));
        } else {
            // Écho du message reçu
            snprintf(rep, sizeof(rep), "Serveur a recu (%zu octets) : %s\n", strlen(buf), buf);
        }

        // Envoi de la réponse
        send(sock, rep, strlen(rep), 0);
    }

    close(sock);
    printf("[%d] Session terminee avec %s:%d\n", getpid(), ip, port);
}
/* Point d'entrée du serveur V1 : écoute les connexions et traite chaque client, éventuellement dans un processus fils. */

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : PORT_DEFAUT;

    // Évite les processus zombies
    signal(SIGCHLD, SIG_IGN);

    // Étape 1 : Création de la socket TCP (IPv4)
    int s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ecoute < 0) { perror("socket"); return 1; }

    // Étape 2 : Réutilisation du port (évite blocage TIME_WAIT)
    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Étape 3 : Attachement (bind) sur toutes les interfaces réseau
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s_ecoute, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    // Étape 4 : Mise en écoute (file d'attente de 5 connexions)
    if (listen(s_ecoute, 5) < 0) { perror("listen"); return 1; }

    afficher_ip(port);

    // Étape 5 : Boucle d'acceptation et fork pour concurrence
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t lg = sizeof(client_addr);
        int s_service = accept(s_ecoute, (struct sockaddr *)&client_addr, &lg);
        if (s_service < 0) continue;

        // Processus fils pour chaque client
        if (fork() == 0) {
            close(s_ecoute); // Le fils ne gère pas l'écoute
            traiter_client(s_service, client_addr);
            exit(0);
        }
        close(s_service); // Le père continue d'écouter
    }

    close(s_ecoute);
    return 0;
}
