/* ==============================================================================
 * EXERCICE 2 - VERSION 2 : Client de transfert multi-données (Fichiers, etc.)
 * Permet d'envoyer et recevoir tout format de données depuis le serveur distant.
 * Usage : ./client_v2 <ip_serveur> [port]
 * ============================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "ftp_common.h"

#define PORT_DEFAUT 5000

// Fonction : Télécharge un fichier depuis le serveur (GET)
/* Demande un fichier distant et le télécharge en blocs dans un fichier local. */
static void cmd_get(int sock, const char *nom) {
    char req[256], line[256];
    snprintf(req, sizeof(req), "GET_REQUEST %s\n", nom);
    send_all(sock, req, strlen(req));

    // Lecture de la réponse : "GET_REPLY OK <taille>" ou "GET_REPLY ERR ..."
    if (recv_line(sock, line, sizeof(line)) < 0) return;
    if (strncmp(line, "GET_REPLY OK ", 13) != 0) {
        printf("  [Erreur] %s\n", line);
        return;
    }

    long long size = atoll(line + 13);
    printf("  [Telechargement] '%s' (%lld octets)...\n", nom, size);

    FILE *f = fopen(nom, "wb"); // Mode binaire pour préserver tout format
    if (!f) { perror("fopen"); return; }

    int r = stream_to_file(sock, f, size);
    fclose(f);

    if (r == 0) printf("  [Succes] Fichier '%s' recu avec succes !\n", nom);
    else printf("  [Erreur] Echec lors de la reception.\n");
}

// Fonction : Téléverse un fichier vers le serveur (PUT)
/* Envoie un fichier local au serveur en indiquant sa taille et son nom. */
static void cmd_put(int sock, const char *chemin) {
    struct stat st;
    if (stat(chemin, &st) < 0 || !S_ISREG(st.st_mode)) {
        printf("  [Erreur] Fichier local introuvable : %s\n", chemin);
        return;
    }

    FILE *f = fopen(chemin, "rb"); // Mode binaire
    if (!f) { perror("fopen"); return; }

    char copie[256];
    snprintf(copie, sizeof(copie), "%s", chemin);
    const char *nom_simple = basename(copie);

    // Annonce le nom et la taille au serveur
    char req[256];
    snprintf(req, sizeof(req), "PUT_REQUEST %s %lld\n", nom_simple, (long long)st.st_size);
    send_all(sock, req, strlen(req));

    printf("  [Envoi] '%s' (%lld octets)...\n", nom_simple, (long long)st.st_size);
    stream_from_file(sock, f, st.st_size);
    fclose(f);

    char rep[256];
    recv_line(sock, rep, sizeof(rep));
    printf("  %s\n", rep);
}

// Fonction : Demande la liste des fichiers au serveur (LS)
/* Demande la liste des fichiers du répertoire distant et l'affiche. */
static void cmd_ls(int sock) {
    char req[] = "LS_REQUEST\n", line[256];
    send_all(sock, req, strlen(req));
    while (recv_line(sock, line, sizeof(line)) >= 0) {
        if (!strcmp(line, "FIN_LS")) break;
        if (strcmp(line, "LS_REPLY OK") != 0) printf("%s\n", line);
    }
}

// Fonction : Change de répertoire distant (CD) ou affiche le chemin (PWD)
/* Envoie une commande textuelle (par exemple pwd ou cd) et affiche la réponse. */
static void cmd_texte(int sock, const char *req_str) {
    char req[256], rep[256];
    snprintf(req, sizeof(req), "%s\n", req_str);
    send_all(sock, req, strlen(req));
    recv_line(sock, rep, sizeof(rep));
    printf("  %s\n", rep);
}
/* Point d'entrée du client V2 : se connecte au serveur puis interprète les commandes saisies. */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage : %s <ip_serveur> [port]\n", argv[0]);
        printf("Exemple : %s 192.168.1.50 5000\n", argv[0]);
        return 1;
    }

    const char *hote = argv[1];
    int port = (argc > 2) ? atoi(argv[2]) : PORT_DEFAUT;

    // Étape 1 : Résolution de l'adresse IP
    struct hostent *host = gethostbyname(hote);
    if (!host) { fprintf(stderr, "Hote inconnu : %s\n", hote); return 1; }

    // Étape 2 : Création de la socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // Étape 3 : Connexion au serveur de fichiers
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons((unsigned short)port);
    memcpy(&saddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    printf("Connexion vers %s:%d...\n", hote, port);
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("connect"); return 1;
    }
    printf("Connecte au serveur de donnees !\n");
    printf("Commandes : put <f>, get <f>, ls, cd <rep>, pwd, quit\n\n");

    char ligne[256];

    // Étape 4 : Boucle d'interaction
    while (1) {
        printf("v2> ");
        fflush(stdout);

        if (!fgets(ligne, sizeof(ligne), stdin)) break;
        ligne[strcspn(ligne, "\r\n")] = '\0';

        char cmd[64] = "", arg[192] = "";
        sscanf(ligne, "%63s %191[^\n]", cmd, arg);

        if (!strcasecmp(cmd, "quit") || !strcasecmp(cmd, "exit")) break;
        else if (!strcasecmp(cmd, "ls")) cmd_ls(sock);
        else if (!strcasecmp(cmd, "pwd")) cmd_texte(sock, "PWD_REQUEST");
        else if (!strcasecmp(cmd, "cd")) {
            char cd_req[256];
            snprintf(cd_req, sizeof(cd_req), "CD_REQUEST %s", arg);
            cmd_texte(sock, cd_req);
        } else if (!strcasecmp(cmd, "get")) cmd_get(sock, arg);
        else if (!strcasecmp(cmd, "put")) cmd_put(sock, arg);
        else if (strlen(cmd) > 0) printf("Commandes valides : put, get, ls, cd, pwd, quit\n");
    }

    close(sock);
    printf("Deconnexion terminee.\n");
    return 0;
}
