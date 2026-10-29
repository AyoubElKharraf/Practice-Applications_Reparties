/* Exercice 1 - Client TCP simple : usage ./client_simple [machine] */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 6000
#define TAILLEBUF 256

int main(int argc, char *argv[]) {
    const char *machine = (argc > 1) ? argv[1] : "localhost";
    struct sockaddr_in addr_serveur;
    struct hostent *host;
    int s_id, nb;
    char reponse[TAILLEBUF];
    const char *message = "Bonjour serveur";

    s_id = socket(AF_INET, SOCK_STREAM, 0);
    if (s_id == -1) { perror("socket"); exit(1); }

    /* identifiants de la machine serveur */
    host = gethostbyname(machine);
    if (host == NULL) { fprintf(stderr, "machine inconnue : %s\n", machine); exit(1); }

    memset(&addr_serveur, 0, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons(PORT);
    memcpy(&addr_serveur.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    if (connect(s_id, (struct sockaddr *)&addr_serveur, sizeof(addr_serveur)) == -1) {
        perror("connect"); exit(1);
    }

    send(s_id, message, strlen(message) + 1, 0);

    nb = recv(s_id, reponse, TAILLEBUF - 1, 0);
    if (nb == -1) { perror("recv"); exit(1); }
    reponse[nb] = '\0';
    printf("Réponse du serveur : %s\n", reponse);

    close(s_id);
    return 0;
}
