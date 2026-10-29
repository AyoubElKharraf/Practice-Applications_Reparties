/* ==============================================================================
 * EXERCICE 2 - VERSION 1 : Client distant (Messages & Commandes)
 * Usage : ./client_v1 <ip_serveur> [port]
 * ============================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_DEFAUT 5000
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage : %s <ip_ou_hote_serveur> [port]\n", argv[0]);
        printf("Exemple : %s 192.168.1.50 5000\n", argv[0]);
        return 1;
    }

    const char *hote = argv[1];
    int port = (argc > 2) ? atoi(argv[2]) : PORT_DEFAUT;

    // Étape 1 : Résolution du nom d'hôte ou IP
    struct hostent *host = gethostbyname(hote);
    if (!host) { fprintf(stderr, "Hote inconnu : %s\n", hote); return 1; }

    // Étape 2 : Création de la socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    // Étape 3 : Configuration de l'adresse du serveur
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((unsigned short)port);
    memcpy(&saddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    // Étape 4 : Connexion au serveur distant
    printf("Connexion vers %s:%d...\n", hote, port);
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    printf("Connecte au serveur distant ! (Tapez 'quit' pour sortir, 'ping', 'date' ou vos messages)\n\n");

    char saisie[BUF_SIZE], reponse[BUF_SIZE];

    // Étape 5 : Boucle interactive d'envoi et de réception
    while (1) {
        printf("v1> ");
        fflush(stdout);

        if (!fgets(saisie, sizeof(saisie), stdin)) break;
        saisie[strcspn(saisie, "\r\n")] = '\0';
        if (strlen(saisie) == 0) continue;

        // Envoi du message au serveur
        send(sock, saisie, strlen(saisie), 0);

        // Si l'utilisateur a tapé 'quit'
        int fini = (!strcasecmp(saisie, "quit"));

        // Réception de la réponse
        int n = recv(sock, reponse, sizeof(reponse) - 1, 0);
        if (n <= 0) { printf("Connexion fermee par le serveur.\n"); break; }

        reponse[n] = '\0';
        printf("%s", reponse);

        if (fini) break;
    }

    // Étape 6 : Fermeture propre de la socket
    close(sock);
    printf("Deconnexion terminee.\n");
    return 0;
}
