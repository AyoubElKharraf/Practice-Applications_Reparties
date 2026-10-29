/* ==============================================================================
 * EXERCICE 2 - Primitives de streaming réseau et framing TCP
 * Permet d'envoyer et recevoir des flux complets sans perte ni tronquage.
 * ============================================================================== */

#ifndef FTP_COMMON_H
#define FTP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define CHUNK 4096

// Fonction : Envoie exactement n octets en bouclant sur send()
static inline int send_all(int fd, const void *buf, size_t n) {
    const char *p = buf;
    while (n > 0) {
        ssize_t w = send(fd, p, n, 0);
        if (w <= 0) return -1;
        p += w;
        n -= (size_t)w;
    }
    return 0;
}

// Fonction : Reçoit exactement n octets en bouclant sur recv()
static inline int recv_all(int fd, void *buf, size_t n) {
    char *p = buf;
    while (n > 0) {
        ssize_t r = recv(fd, p, n, 0);
        if (r <= 0) return -1;
        p += r;
        n -= (size_t)r;
    }
    return 0;
}

// Fonction : Lit une ligne terminée par '\n' (pour lire les en-têtes texte)
static inline int recv_line(int fd, char *buf, size_t max) {
    size_t i = 0;
    char c;
    while (i < max - 1) {
        ssize_t r = recv(fd, &c, 1, 0);
        if (r <= 0) return -1;
        if (c == '\n') { buf[i] = '\0'; return (int)i; }
        if (c != '\r') buf[i++] = c;
    }
    return -1;
}

// Fonction : Transfère 'size' octets de la socket vers un fichier
static inline int stream_to_file(int fd, FILE *f, long long size) {
    char buf[CHUNK];
    while (size > 0) {
        size_t k = (size > CHUNK) ? CHUNK : (size_t)size;
        if (recv_all(fd, buf, k) < 0) return -1;
        if (f && fwrite(buf, 1, k, f) != k) return 1;
        size -= (long long)k;
    }
    return 0;
}

// Fonction : Transfère 'size' octets d'un fichier vers la socket
static inline int stream_from_file(int fd, FILE *f, long long size) {
    char buf[CHUNK];
    while (size > 0) {
        size_t k = (size > CHUNK) ? CHUNK : (size_t)size;
        size_t r = fread(buf, 1, k, f);
        if (r == 0) return -1;
        if (send_all(fd, buf, r) < 0) return -1;
        size -= (long long)r;
    }
    return 0;
}

#endif
