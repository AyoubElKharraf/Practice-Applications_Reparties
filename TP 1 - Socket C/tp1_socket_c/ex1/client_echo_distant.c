/* Exercice 1 - Client écho TCP distant
 * Usage : ./client_echo_distant <ip_ou_nom_du_serveur> [port]
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
        fprintf(stderr, " Usage : %s <ip_ou_nom_du_serveur> [port]\n", argv[0]);
        fprintf(stderr, " Exemples :\n");
        fprintf(stderr, "   %s 192.168.1.50\n", argv[0]);
        fprintf(stderr, "   %s 192.168.1.50 6000\n", argv[0]);
        fprintf(stderr, "=================================================================\n");
        return 1;
    }

    const char *machine = argv[1];
    int port = (argc > 2) ? atoi(argv[2]) : PORT_DEFAUT;

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Erreur : port invalide '%s'\n", argv[2]);
        return 1;
    }

    struct sockaddr_in addr_serveur;
    struct hostent *host;
    int s_id, nb;
    char phrase[TAILLEBUF], reponse[TAILLEBUF];

    /* 1. Création de la socket */
    s_id = socket(AF_INET, SOCK_STREAM, 0);
    if (s_id == -1) {
        perror("socket");
        return 1;
    }

    /* 2. Résolution d'adresse */
    printf("[CLIENT ECHO DISTANT] Résolution de l'adresse '%s'...\n", machine);
    host = gethostbyname(machine);
    if (host == NULL) {
        fprintf(stderr, "Erreur : machine ou IP inconnue : %s\n", machine);
        close(s_id);
        return 1;
    }

    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons((unsigned short)port);
    memcpy(&addr_serveur.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    printf("[CLIENT ECHO DISTANT] Connexion vers %s (%s):%d...\n",
           machine, inet_ntoa(addr_serveur.sin_addr), port);

    /* 3. Connexion */
    if (connect(s_id, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("connect (Impossible de joindre le serveur distant)");
        close(s_id);
        return 1;
    }
    printf("[CLIENT ECHO DISTANT] Connecté au serveur !\n");
    printf("Tapez vos messages (tapez 'quit' pour quitter) :\n");

    /* 4. Boucle d'envoi / réception interactive */
    while (printf("> "), fflush(stdout), fgets(phrase, TAILLEBUF, stdin) != NULL) {
        phrase[strcspn(phrase, "\r\n")] = '\0';
        if (phrase[0] == '\0') continue;

        if (send(s_id, phrase, strlen(phrase), 0) == -1) {
            perror("send");
            break;
        }

        if (strcmp(phrase, "quit") == 0) {
            printf("[CLIENT ECHO DISTANT] Fin demandée. Déconnexion...\n");
            break;
        }

        nb = recv(s_id, reponse, TAILLEBUF - 1, 0);
        if (nb <= 0) {
            printf("[CLIENT ECHO DISTANT] Connexion fermée par le serveur distant.\n");
            break;
        }
        reponse[nb] = '\0';
        printf("Serveur distant : %s\n", reponse);
    }

    close(s_id);
    return 0;
}

