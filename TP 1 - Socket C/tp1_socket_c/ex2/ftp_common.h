/* Fonctions communes client/serveur FTP.
 *
 * PROTOCOLE (texte pour l'en-tête, binaire pour les données) :
 *   Requête  : "<NOM>_REQUEST [argument]\n"
 *              PUT : "PUT_REQUEST <nom_fichier> <taille>\n" suivi de <taille> octets
 *   Réponse  : "<NOM>_REPLY <OK|ERR> <longueur>\n" suivi de <longueur> octets
 *              (le contenu du fichier pour GET, un texte pour les autres commandes
 *               ou le message d'erreur si ERR)
 * Annoncer la taille avant les données est indispensable : TCP est un FLOT d'octets,
 * il ne conserve pas les frontières des messages.
 */
#ifndef FTP_COMMON_H
#define FTP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define LINE_MAX_LEN 1024
#define CHUNK        4096

/* send() peut n'envoyer qu'une partie des octets : on boucle jusqu'au bout */
static int send_all(int fd, const void *buf, size_t n) {
    const char *p = buf;
    while (n > 0) {
        ssize_t w = send(fd, p, n, MSG_NOSIGNAL);
        if (w < 0 && errno == EINTR) continue;
        if (w <= 0) return -1;
        p += w;
        n -= (size_t)w;
    }
    return 0;
}

/* recv() peut renvoyer moins que demandé : on boucle jusqu'à avoir n octets */
static int recv_all(int fd, void *buf, size_t n) {
    char *p = buf;
    while (n > 0) {
        ssize_t r = recv(fd, p, n, 0);
        if (r < 0 && errno == EINTR) continue;
        if (r <= 0) return -1;          /* 0 = connexion fermée par le pair */
        p += r;
        n -= (size_t)r;
    }
    return 0;
}

/* lit une ligne terminée par '\n' (retirée). -1 si connexion fermée ou ligne trop longue */
static int recv_line(int fd, char *buf, size_t max) {
    size_t i = 0;
    char c;
    while (i < max - 1) {
        ssize_t r = recv(fd, &c, 1, 0);
        if (r < 0 && errno == EINTR) continue;
        if (r <= 0) return -1;
        if (c == '\n') { buf[i] = '\0'; return (int)i; }
        if (c != '\r') buf[i++] = c;
    }
    return -1;
}

/* envoie "<tag> OK|ERR <len>\n" puis les données */
static __attribute__((unused)) int send_reply(int fd, const char *tag, int ok, const void *data, size_t len) {
    char hdr[128];
    snprintf(hdr, sizeof hdr, "%s %s %zu\n", tag, ok ? "OK" : "ERR", len);
    if (send_all(fd, hdr, strlen(hdr)) < 0) return -1;
    if (len > 0 && send_all(fd, data, len) < 0) return -1;
    return 0;
}

/* socket -> fichier : lit exactement n octets. Si f == NULL, les octets sont jetés
 * (indispensable pour rester synchronisé avec le flot même quand on refuse l'opération).
 * Retour : 0 ok, 1 erreur d'écriture locale (flot quand même vidé), -1 erreur réseau */
static __attribute__((unused)) int stream_to_file(int fd, FILE *f, long long n) {
    char buf[CHUNK];
    int werr = 0;
    while (n > 0) {
        size_t k = (n > CHUNK) ? CHUNK : (size_t)n;
        if (recv_all(fd, buf, k) < 0) return -1;
        if (f && !werr && fwrite(buf, 1, k, f) != k) werr = 1;
        n -= (long long)k;
    }
    if (f && !werr && fflush(f) != 0) werr = 1;
    return werr;
}

/* fichier -> socket : envoie exactement n octets lus dans f */
static __attribute__((unused)) int stream_from_file(int fd, FILE *f, long long n) {
    char buf[CHUNK];
    while (n > 0) {
        size_t k = (n > CHUNK) ? CHUNK : (size_t)n;
        size_t r = fread(buf, 1, k, f);
        if (r == 0) return -1;          /* fichier tronqué pendant l'envoi */
        if (send_all(fd, buf, r) < 0) return -1;
        n -= (long long)r;
    }
    return 0;
}

/* Affichage d'une barre de progression interactive dans le terminal */
static __attribute__((unused)) void afficher_progression(long long courant, long long total, const char *label) {
    int largeur = 30;
    float ratio = (total > 0) ? (float)courant / (float)total : 1.0f;
    int pos = (int)(ratio * largeur);
    if (pos > largeur) pos = largeur;
    printf("\r  %s [", label ? label : "Transfert");
    for (int i = 0; i < largeur; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %3d%% (%.2f / %.2f Mo)",
           (int)(ratio * 100),
           (double)courant / (1024.0 * 1024.0),
           (double)total / (1024.0 * 1024.0));
    fflush(stdout);
    if (courant >= total) printf("\n");
}

/* Réception avec barre de progression */
static __attribute__((unused)) int stream_to_file_progress(int fd, FILE *f, long long total, const char *label) {
    char buf[CHUNK];
    int werr = 0;
    long long restant = total;
    long long recu = 0;

    afficher_progression(0, total, label);
    while (restant > 0) {
        size_t k = (restant > CHUNK) ? CHUNK : (size_t)restant;
        if (recv_all(fd, buf, k) < 0) return -1;
        if (f && !werr && fwrite(buf, 1, k, f) != k) werr = 1;
        restant -= (long long)k;
        recu += (long long)k;
        if (total > 102400) { /* Rafraîchit si > 100 Ko */
            afficher_progression(recu, total, label);
        }
    }
    afficher_progression(total, total, label);
    if (f && !werr && fflush(f) != 0) werr = 1;
    return werr;
}

/* Envoi avec barre de progression */
static __attribute__((unused)) int stream_from_file_progress(int fd, FILE *f, long long total, const char *label) {
    char buf[CHUNK];
    long long restant = total;
    long long envoye = 0;

    afficher_progression(0, total, label);
    while (restant > 0) {
        size_t k = (restant > CHUNK) ? CHUNK : (size_t)restant;
        size_t r = fread(buf, 1, k, f);
        if (r == 0) return -1;
        if (send_all(fd, buf, r) < 0) return -1;
        restant -= (long long)r;
        envoye += (long long)r;
        if (total > 102400) {
            afficher_progression(envoye, total, label);
        }
    }
    afficher_progression(total, total, label);
    return 0;
}

#endif
