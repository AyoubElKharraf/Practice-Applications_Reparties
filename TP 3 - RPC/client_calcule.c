#include <stdio.h>
#include <stdlib.h>
#include "calcule.h"

int main(int argc, char *argv[]) {
    CLIENT *client;
    operandes param;
    float *resultat;
    int n, *p;
    char *server_host = "localhost";

    /* Si l'utilisateur fournit l'adresse/nom de l'hôte en argument */
    if (argc >= 2) {
        server_host = argv[1];
    }

    /* Connexion au service RPC distant via UDP */
    client = clnt_create(server_host, MATHS_PROG, MATHS_VERSION_1, "udp");
    if (client == NULL) {
        clnt_pcreateerror(server_host);
        fprintf(stderr, "Erreur lors de la creation du client RPC vers %s\n", server_host);
        exit(EXIT_FAILURE);
    }

    /* 1. Appel de la fonction distante multxy_1 */
    param.x = 12.0f;
    param.y = 20.0f;
    resultat = multxy_1(&param, client);
    if (resultat == (float *) NULL) {
        clnt_perror(client, "Echec de l'appel RPC multxy_1");
    } else {
        printf("Calcul distant : multxy(%.2f, %.2f) = %f\n", param.x, param.y, *resultat);
    }

    /* 2. Appel de la fonction distante puiss2_1 */
    n = 16;
    p = puiss2_1(&n, client);
    if (p == (int *) NULL) {
        clnt_perror(client, "Echec de l'appel RPC puiss2_1");
    } else {
        printf("Calcul distant : puiss2(%d) = %d\n", n, *p);
    }

    /* Nettoyage du handle client */
    clnt_destroy(client);

    return 0;
}
