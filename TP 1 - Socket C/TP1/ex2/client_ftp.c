/* Client FTP (TP1, exercice 2)
 *   Étape 1 (une commande par lancement) :
 *       ./client_ftp <machine> <port> GET_REQUEST toto.txt
 *       ./client_ftp <machine> <port> PUT_REQUEST popo.txt   (idem CD_REQUEST, LS_REQUEST, PWD_REQUEST)
 *   Étape 2 (shell interactif, UNE seule connexion TCP) :
 *       ./client_ftp <machine> <port>
 *       ftp> ls | pwd | cd <rep> | get <fichier> | put <fichier> | quit */
#include <ctype.h>
#include <libgen.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include "ftp_common.h"

#define MAX_TEXT (16 * 1024 * 1024)

/* lit l'en-tête de réponse : "<tag> OK|ERR <len>" */
static int read_reply(int fd, int *ok, long long *len) {
    char line[LINE_MAX_LEN], tag[64], st[8];
    if (recv_line(fd, line, sizeof line) < 0) return -1;
    if (sscanf(line, "%63s %7s %lld", tag, st, len) != 3 || *len < 0) return -1;
    *ok = (strcmp(st, "OK") == 0);
    return 0;
}

/* lit <len> octets de texte et les affiche (préfixe "Erreur : " si erreur) */
static int print_payload(int fd, int ok, long long len) {
    if (len > MAX_TEXT) return -1;
    char *buf = malloc((size_t)len + 1);
    if (!buf) return -1;
    if (len > 0 && recv_all(fd, buf, (size_t)len) < 0) { free(buf); return -1; }
    buf[len] = '\0';
    if (!ok) printf("Erreur : ");
    printf("%s", buf);
    if (len == 0 || buf[len - 1] != '\n') printf("\n");
    free(buf);
    return 0;
}

/* commandes "texte" : ls, pwd, cd */
static int cmd_text(int fd, const char *request, const char *arg) {
    char line[LINE_MAX_LEN];
    int ok; long long len;
    snprintf(line, sizeof line, "%s%s%s\n", request, arg[0] ? " " : "", arg);
    if (send_all(fd, line, strlen(line)) < 0) return -1;
    if (read_reply(fd, &ok, &len) < 0) return -1;
    return print_payload(fd, ok, len);
}

static int cmd_get(int fd, const char *name) {
    char line[LINE_MAX_LEN];
    int ok; long long len;
    if (name[0] == '\0') { printf("Usage : get <fichier>\n"); return 0; }
    snprintf(line, sizeof line, "GET_REQUEST %s\n", name);
    if (send_all(fd, line, strlen(line)) < 0) return -1;
    if (read_reply(fd, &ok, &len) < 0) return -1;
    if (!ok) return print_payload(fd, 0, len);

    char copy[PATH_MAX];
    snprintf(copy, sizeof copy, "%s", name);
    FILE *f = fopen(basename(copy), "wb");               /* stocké dans le répertoire courant du client */
    if (!f) printf("Erreur locale : %s\n", strerror(errno));
    int r = stream_to_file(fd, f, len);                  /* f == NULL : on vide quand même le flot */
    if (f) fclose(f);
    if (r < 0) return -1;
    if (f && r == 0) printf("Fichier '%s' recu (%lld octets)\n", name, len);
    else if (f) printf("Erreur d'ecriture locale\n");
    return 0;
}

static int cmd_put(int fd, const char *name) {
    char line[LINE_MAX_LEN], copy[PATH_MAX];
    int ok; long long len;
    struct stat st;
    if (name[0] == '\0') { printf("Usage : put <fichier>\n"); return 0; }
    if (stat(name, &st) < 0 || !S_ISREG(st.st_mode)) { printf("Erreur locale : fichier introuvable\n"); return 0; }
    FILE *f = fopen(name, "rb");
    if (!f) { printf("Erreur locale : %s\n", strerror(errno)); return 0; }

    snprintf(copy, sizeof copy, "%s", name);
    snprintf(line, sizeof line, "PUT_REQUEST %s %lld\n", basename(copy), (long long)st.st_size);
    int r = send_all(fd, line, strlen(line));
    if (r == 0) r = stream_from_file(fd, f, (long long)st.st_size);
    fclose(f);
    if (r < 0) return -1;
    if (read_reply(fd, &ok, &len) < 0) return -1;
    return print_payload(fd, ok, len);
}

static int connecter(const char *machine, int port) {
    struct hostent *host = gethostbyname(machine);
    if (!host) { fprintf(stderr, "machine inconnue : %s\n", machine); return -1; }
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) { perror("socket"); return -1; }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    if (connect(s, (struct sockaddr *)&addr, sizeof addr) == -1) { perror("connect"); close(s); return -1; }
    return s;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage : %s <machine> <port> [COMMANDE_REQUEST [argument]]\n", argv[0]);
        return 1;
    }
    int fd = connecter(argv[1], atoi(argv[2]));
    if (fd < 0) return 1;
    int r = 0;

    if (argc >= 4) {                                     /* ---- étape 1 : une commande ---- */
        const char *cmd = argv[3], *arg = (argc >= 5) ? argv[4] : "";
        if      (!strcmp(cmd, "GET_REQUEST")) r = cmd_get(fd, arg);
        else if (!strcmp(cmd, "PUT_REQUEST")) r = cmd_put(fd, arg);
        else if (!strcmp(cmd, "CD_REQUEST") || !strcmp(cmd, "LS_REQUEST") || !strcmp(cmd, "PWD_REQUEST"))
            r = cmd_text(fd, cmd, arg);
        else fprintf(stderr, "commande inconnue : %s\n", cmd);
    } else {                                             /* ---- étape 2 : shell interactif ---- */
        char input[LINE_MAX_LEN];
        while (printf("ftp> "), fflush(stdout), fgets(input, sizeof input, stdin) != NULL) {
            input[strcspn(input, "\r\n")] = '\0';
            char *cmd = input;
            while (isspace((unsigned char)*cmd)) cmd++;
            char *arg = cmd;
            while (*arg && !isspace((unsigned char)*arg)) arg++;
            if (*arg) *arg++ = '\0';
            while (isspace((unsigned char)*arg)) arg++;

            if (cmd[0] == '\0') continue;
            if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit")) break;
            else if (!strcmp(cmd, "ls"))  r = cmd_text(fd, "LS_REQUEST", arg);
            else if (!strcmp(cmd, "pwd")) r = cmd_text(fd, "PWD_REQUEST", "");
            else if (!strcmp(cmd, "cd"))  r = cmd_text(fd, "CD_REQUEST", arg);
            else if (!strcmp(cmd, "get")) r = cmd_get(fd, arg);
            else if (!strcmp(cmd, "put")) r = cmd_put(fd, arg);
            else { printf("Commandes : ls [rep], pwd, cd <rep>, get <f>, put <f>, quit\n"); r = 0; }
            if (r < 0) break;
        }
    }
    if (r < 0) fprintf(stderr, "Connexion perdue ou erreur de protocole\n");
    close(fd);
    return r < 0 ? 1 : 0;
}
