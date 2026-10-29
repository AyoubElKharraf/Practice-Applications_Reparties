/* ==============================================================================
 * EXERCICE 2 - VERSION 2 : Serveur de transfert multi-données (Fichiers, etc.)
 * Permet de transférer tout format de données (binaires, vidéos, images, textes).
 * Usage : ./serveur_v2 [port] [dossier_racine]
 * ============================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include "ftp_common.h"

#define PORT_DEFAUT 5000

static char root[PATH_MAX]; // Dossier racine confiné (sandbox)

// Fonction : Affiche les IP réseau pour la connexion cliente distante
static void afficher_ip(int port, const char *dossier) {
    struct ifaddrs *ifaddr, *ifa;
    printf("[SERVEUR V2] Transfert de fichiers actif sur le port %d\n", port);
    printf("Racine de stockage : %s\n", dossier);
    printf("Adresses pour connecter la Machine 2 (Client distant) :\n");
    if (getifaddrs(&ifaddr) != -1) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in *p = (struct sockaddr_in *)ifa->ifa_addr;
                char *ip = inet_ntoa(p->sin_addr);
                if (strcmp(ip, "127.0.0.1") != 0)
                    printf("  -> IP : %s | Commande : ./client_v2 %s %d\n", ip, ip, port);
            }
        }
        freeifaddrs(ifaddr);
    }
    printf("  -> Local : 127.0.0.1 (./client_v2 127.0.0.1 %d)\n\n", port);
}

// Fonction : Empêche de sortir de la racine (sécurité sandbox)
static int in_root(const char *path) {
    size_t rl = strlen(root);
    return (strncmp(path, root, rl) == 0 && (rl == 1 || path[rl] == '\0' || path[rl] == '/'));
}

// Fonction : Téléchargement vers le client (GET) - Mode binaire
static int handle_get(int fd, const char *name) {
    struct stat st;
    if (strchr(name, '/') || stat(name, &st) < 0 || !S_ISREG(st.st_mode)) {
        char err[] = "GET_REPLY ERR Fichier introuvable\n";
        return send_all(fd, err, strlen(err));
    }
    FILE *f = fopen(name, "rb"); // "rb" pour tout type (video, pdf, image)
    if (!f) return -1;

    // Annonce la taille au client avant d'envoyer les octets
    char hdr[128];
    snprintf(hdr, sizeof(hdr), "GET_REPLY OK %lld\n", (long long)st.st_size);
    send_all(fd, hdr, strlen(hdr));

    printf("[%d] Envoi du fichier '%s' (%lld octets)...\n", getpid(), name, (long long)st.st_size);
    int r = stream_from_file(fd, f, st.st_size);
    fclose(f);
    return r;
}

// Fonction : Téléversement depuis le client (PUT) - Mode binaire
static int handle_put(int fd, char *args) {
    char *sp = strrchr(args, ' ');
    if (!sp) return -1;
    *sp = '\0';
    long long size = atoll(sp + 1);

    FILE *f = fopen(args, "wb"); // "wb" pour tout type de données
    printf("[%d] Reception du fichier '%s' (%lld octets)...\n", getpid(), args, size);

    // Lit exactement 'size' octets depuis la socket TCP
    int r = stream_to_file(fd, f, size);
    if (f) fclose(f);

    char rep[128];
    if (r == 0) snprintf(rep, sizeof(rep), "PUT_REPLY OK Fichier '%s' enregistre avec succes\n", args);
    else snprintf(rep, sizeof(rep), "PUT_REPLY ERR Erreur lors de l'enregistrement\n");
    return send_all(fd, rep, strlen(rep));
}

// Fonction : Liste les fichiers du dossier courant (LS)
static int handle_ls(int fd) {
    struct dirent **list;
    int n = scandir(".", &list, NULL, alphasort);
    if (n < 0) return -1;

    char out[8192] = "LS_REPLY OK\n";
    struct stat st;
    for (int i = 0; i < n; i++) {
        if (strcmp(list[i]->d_name, ".") != 0 && strcmp(list[i]->d_name, "..") != 0) {
            char ligne[512];
            stat(list[i]->d_name, &st);
            if (S_ISDIR(st.st_mode))
                snprintf(ligne, sizeof(ligne), "  [DOSSIER] %s/\n", list[i]->d_name);
            else
                snprintf(ligne, sizeof(ligne), "  [FICHIER] %-20s (%lld octets)\n", list[i]->d_name, (long long)st.st_size);
            strcat(out, ligne);
        }
        free(list[i]);
    }
    free(list);
    strcat(out, "FIN_LS\n");
    return send_all(fd, out, strlen(out));
}

// Fonction : Change de répertoire (CD) avec vérification de confinement
static int handle_cd(int fd, const char *rep) {
    char cur[PATH_MAX], rep_msg[PATH_MAX + 64];
    if (chdir(rep) == 0 && getcwd(cur, sizeof(cur)) && in_root(cur)) {
        snprintf(rep_msg, sizeof(rep_msg), "CD_REPLY OK Repertoire : %s\n", cur);
    } else {
        chdir(root); // Rétablit la racine si tentative de sortie
        snprintf(rep_msg, sizeof(rep_msg), "CD_REPLY ERR Acces refuse hors de la racine\n");
    }
    return send_all(fd, rep_msg, strlen(rep_msg));
}

// Fonction : Session de transfert avec un client distant (processus fils)
static void session(int fd) {
    char line[512];
    while (recv_line(fd, line, sizeof(line)) >= 0) {
        char *arg = strchr(line, ' ');
        if (arg) *arg++ = '\0'; else arg = "";

        if (!strcmp(line, "GET_REQUEST")) handle_get(fd, arg);
        else if (!strcmp(line, "PUT_REQUEST")) handle_put(fd, arg);
        else if (!strcmp(line, "LS_REQUEST"))  handle_ls(fd);
        else if (!strcmp(line, "CD_REQUEST"))  handle_cd(fd, arg);
        else if (!strcmp(line, "PWD_REQUEST")) {
            char cur[PATH_MAX], rep[PATH_MAX + 32];
            getcwd(cur, sizeof(cur));
            snprintf(rep, sizeof(rep), "PWD_REPLY OK %s\n", cur);
            send_all(fd, rep, strlen(rep));
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : PORT_DEFAUT;
    const char *dossier = (argc > 2) ? argv[2] : "./racine_ftp";

    // Évite les zombies et ignore SIGPIPE
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    // Initialisation du dossier racine
    mkdir(dossier, 0755);
    chdir(dossier);
    getcwd(root, sizeof(root));

    // Étape 1 : Création de la socket TCP
    int s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Étape 2 : Attachement (bind)
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s_ecoute, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    // Étape 3 : Mise en écoute
    if (listen(s_ecoute, 5) < 0) { perror("listen"); return 1; }

    afficher_ip(port, root);

    // Étape 4 : Boucle d'acceptation et fork
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t lg = sizeof(client_addr);
        int s_service = accept(s_ecoute, (struct sockaddr *)&client_addr, &lg);
        if (s_service < 0) continue;

        if (fork() == 0) {
            close(s_ecoute);
            session(s_service);
            exit(0);
        }
        close(s_service);
    }
    close(s_ecoute);
    return 0;
}
