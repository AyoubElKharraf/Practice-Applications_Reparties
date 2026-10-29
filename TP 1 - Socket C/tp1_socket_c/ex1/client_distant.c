/* Exercice 1 - Client TCP pour communication à distance (deux machines distinctes)
 * Usage : ./client_distant <ip_ou_nom_du_serveur> [port] [message]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT_DEFAUT 6000
#define TAILLEBUF 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "=================================================================\n");
        fprintf(stderr, " Usage : %s <ip_ou_nom_du_serveur> [port] [message]\n", argv[0]);
        fprintf(stderr, " Exemples :\n");
        fprintf(stderr, "   %s 192.168.1.50\n", argv[0]);
        fprintf(stderr, "   %s 192.168.1.50 6000\n", argv[0]);
        fprintf(stderr, "   %s 192.168.1.50 6000 \"Bonjour depuis la machine cliente !\"\n", argv[0]);
        fprintf(stderr, "=================================================================\n");
        return 1;
    }

    const char *machine = argv[1];
    int port = (argc > 2) ? atoi(argv[2]) : PORT_DEFAUT;
    const char *message = (argc > 3) ? argv[3] : "Bonjour serveur distant !";

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Erreur : port invalide '%s' (doit être compris entre 1 et 65535)\n", argv[2]);
        return 1;
    }

    struct sockaddr_in addr_serveur;
    struct hostent *host;
    int s_id, nb;
    char reponse[TAILLEBUF];

    /* 1. Création de la socket client (IPv4, TCP) */
    s_id = socket(AF_INET, SOCK_STREAM, 0);
    if (s_id == -1) {
        perror("socket");
        return 1;
    }

    /* 2. Résolution du nom d'hôte ou de l'adresse IP de la machine distante */
    printf("[CLIENT DISTANT] Résolution de l'adresse distante '%s'...\n", machine);
    host = gethostbyname(machine);
    if (host == NULL) {
        fprintf(stderr, "Erreur : machine ou IP distante inconnue : %s\n", machine);
        close(s_id);
        return 1;
    }

    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons((unsigned short)port);
    memcpy(&addr_serveur.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    printf("[CLIENT DISTANT] Tentative de connexion vers %s (%s) sur le port %d...\n",
           machine, inet_ntoa(addr_serveur.sin_addr), port);

    /* 3. Connexion à la machine distante */
    if (connect(s_id, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("connect (Échec de la connexion. Vérifiez l'adresse IP, le port et les pare-feux)");
        close(s_id);
        return 1;
    }
    printf("[CLIENT DISTANT] Connexion établie avec succès !\n");

    /* 4. Envoi du message au serveur distant */
    printf("[CLIENT DISTANT] Envoi du message : \"%s\"\n", message);
    if (send(s_id, message, strlen(message) + 1, 0) == -1) {
        perror("send");
        close(s_id);
        return 1;
    }

    /* 5. Réception de la réponse du serveur distant */
    nb = recv(s_id, reponse, TAILLEBUF - 1, 0);
    if (nb == -1) {
        perror("recv");
        close(s_id);
        return 1;
    } else if (nb == 0) {
        printf("Le serveur distant a fermé la connexion avant d'envoyer sa réponse.\n");
    } else {
        reponse[nb] = '\0';
        printf("[CLIENT DISTANT] Réponse reçue du serveur distant : \"%s\"\n", reponse);
    }

    /* 6. Fermeture propre de la connexion */
    close(s_id);
    printf("[CLIENT DISTANT] Déconnexion propre terminée.\n");
    return 0;
}

