/* Exercice 1 - Client écho : usage ./client_echo [machine]
 * Lit des phrases au clavier, affiche la réponse, s'arrête sur "quit". */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 6000
#define TAILLEBUF 1024
/* Point d'entrée : se connecte au serveur d'écho et échange des phrases saisies jusqu'à quit. */

int main(int argc, char *argv[]) {
    const char *machine = (argc > 1) ? argv[1] : "localhost";
    struct sockaddr_in addr_serveur;
    struct hostent *host;
    int s_id, nb;
    char phrase[TAILLEBUF], reponse[TAILLEBUF];

    s_id = socket(AF_INET, SOCK_STREAM, 0);
    if (s_id == -1) { perror("socket"); exit(1); }

    host = gethostbyname(machine);
    if (host == NULL) { fprintf(stderr, "machine inconnue : %s\n", machine); exit(1); }

    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(PORT);
    memcpy(&addr_serveur.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    if (connect(s_id, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("connect"); exit(1);
    }

    while (printf("> "), fflush(stdout), fgets(phrase, TAILLEBUF, stdin) != NULL) {
        phrase[strcspn(phrase, "\r\n")] = '\0';
        if (phrase[0] == '\0') continue;

        send(s_id, phrase, strlen(phrase), 0);
        if (strcmp(phrase, "quit") == 0) break;      /* mot-clé de fin */

        nb = recv(s_id, reponse, TAILLEBUF - 1, 0);
        if (nb <= 0) { printf("connexion fermée par le serveur\n"); break; }
        reponse[nb] = '\0';
        printf("Serveur : %s\n", reponse);
    }
    close(s_id);
    return 0;
}
