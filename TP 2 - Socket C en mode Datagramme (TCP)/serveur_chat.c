/* TP2 - Mini chat en mode datagramme (UDP) : SERVEUR
 *
 * Usage : ./serveur_chat <port>
 *
 * Principe :
 *  - le serveur lit des datagrammes sur une seule socket UDP (recvfrom) ;
 *  - un client est "en ligne" dès qu'il a envoyé un message : le serveur mémorise
 *    son adresse (IP + port) dans une table de 10 entrées maximum ;
 *  - chaque message est redistribué (sendto) à tous les AUTRES clients en ligne,
 *    précédé de l'adresse de l'expéditeur : "[ip:port]> message".
 *
 * Extension (hors sujet) : un client qui envoie "/quit" est retiré de la liste,
 * ce qui libère une place pour un autre client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define MSG_MAX     225   /* taille maximale d'un message                              */
#define BUF_SIZE    256   /* tampon : 225 + préfixe "[ip:port]> " (25 max) + '\0' = 251 */
#define CMD_QUIT    "/quit"
#define ADDR_SIZE   22    /* "255.255.255.255:65535" = 21 caractères + '\0' */

static struct sockaddr_in clients[MAX_CLIENTS];   /* adresses des clients en ligne */
static int nb_clients = 0;

/* "a.b.c.d:port" (inet_ntop, contrairement à inet_ntoa, n'utilise pas de tampon statique) */
static void addr_to_str(const struct sockaddr_in *a, char *out, size_t n) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &a->sin_addr, ip, sizeof ip);
    snprintf(out, n, "%s:%d", ip, ntohs(a->sin_port));
}

/* indice du client dans la table, ou -1 s'il n'y est pas */
static int find_client(const struct sockaddr_in *a) {
    for (int i = 0; i < nb_clients; i++)
        if (clients[i].sin_addr.s_addr == a->sin_addr.s_addr &&
            clients[i].sin_port == a->sin_port)
            return i;
    return -1;
}

static void remove_client(int idx) {
    for (int i = idx; i < nb_clients - 1; i++)
        clients[i] = clients[i + 1];
    nb_clients--;
}

static void print_clients(void) {
    char s[ADDR_SIZE];
    printf("Liste des clients en ligne :\n");
    for (int i = 0; i < nb_clients; i++) {
        addr_to_str(&clients[i], s, sizeof s);
        printf("[%s]\n", s);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "Usage : %s <port>\n", argv[0]); return 1; }
    int port = atoi(argv[1]);
    if (port < 1 || port > 65535) { fprintf(stderr, "port invalide : %s\n", argv[1]); return 1; }

    /* 1. création de la socket UDP */
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) { perror("socket"); return 1; }
    printf("Socket créée avec succès ! (%d)\n", s);

    /* 2. nommage : le serveur doit être joignable sur un port connu des clients */
    struct sockaddr_in addr_serveur;
    memset(&addr_serveur, 0, sizeof addr_serveur);
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons((unsigned short)port);
    addr_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&addr_serveur, sizeof addr_serveur) == -1) {
        perror("bind");
        return 1;
    }
    printf("Socket attachée avec succès !\n");
    fflush(stdout);

    /* 3. boucle principale : recevoir un message, le redistribuer */
    while (1) {
        struct sockaddr_in addr_client;
        socklen_t lg = sizeof addr_client;     /* à réinitialiser avant chaque recvfrom */
        char msg[MSG_MAX + 1];                 /* 225 caractères + '\0' */

        /* un message plus long que MSG_MAX est tronqué par le système (cf. cours) */
        ssize_t n = recvfrom(s, msg, MSG_MAX, 0, (struct sockaddr *)&addr_client, &lg);
        if (n < 0) { perror("recvfrom"); continue; }
        msg[n] = '\0';
        msg[strcspn(msg, "\r\n")] = '\0';      /* retire un éventuel retour à la ligne */
        if (msg[0] == '\0') continue;          /* message vide : ignoré */

        char from[ADDR_SIZE];
        addr_to_str(&addr_client, from, sizeof from);
        int idx = find_client(&addr_client);

        /* extension : départ d'un client */
        if (strcmp(msg, CMD_QUIT) == 0) {
            if (idx >= 0) {
                remove_client(idx);
                printf("Le client [%s] a quitté le chat\n", from);
                print_clients();
                fflush(stdout);
            }
            continue;
        }

        /* nouveau client : on l'ajoute à la liste s'il reste de la place */
        if (idx < 0) {
            if (nb_clients == MAX_CLIENTS) {
                const char *refus = "Serveur plein : 10 clients en ligne maximum";
                printf("Message refusé de [%s] : %d clients en ligne maximum\n", from, MAX_CLIENTS);
                sendto(s, refus, strlen(refus), 0, (struct sockaddr *)&addr_client, lg);
                fflush(stdout);
                continue;
            }
            clients[nb_clients++] = addr_client;
            idx = nb_clients - 1;
        }

        printf("Message : %s\n-> reçu du client [%s]\n", msg, from);

        /* message envoyé aux autres : "[ip:port]> texte" */
        char out[BUF_SIZE];
        snprintf(out, sizeof out, "[%s]> %s", from, msg);

        int envoyes = 0;
        for (int i = 0; i < nb_clients; i++) {
            if (i == idx) continue;            /* pas de renvoi à l'expéditeur */
            char dest[ADDR_SIZE];
            addr_to_str(&clients[i], dest, sizeof dest);
            if (sendto(s, out, strlen(out), 0,
                       (struct sockaddr *)&clients[i], sizeof clients[i]) == -1) {
                perror("sendto");
                continue;
            }
            printf("Message : %s\n-> envoyé à [%s]\n", msg, dest);
            envoyes++;
        }
        if (envoyes > 0)
            printf("Le message a été envoyé à %d client(s)\n", envoyes);
        print_clients();
        fflush(stdout);
    }
}
