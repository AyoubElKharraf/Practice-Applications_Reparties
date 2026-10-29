/* Serveur FTP simple (TP1, exercice 2, étapes 1 et 2)
 * Usage : ./serveur_ftp <port> <racine>
 * Un processus fils par client : chdir() est propre à chaque processus, donc
 * chaque client a SON répertoire courant, indépendant des autres. */
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ftp_common.h"

static char root[PATH_MAX];     /* racine réelle gérée par le serveur */

/* path est-il la racine ou un de ses sous-répertoires ? */
static int in_root(const char *path) {
    size_t rl = strlen(root);
    if (strncmp(path, root, rl) != 0) return 0;
    return rl == 1 || path[rl] == '\0' || path[rl] == '/';
}

/* chemin affiché au client : relatif à la racine, "/" = racine */
static const char *virtual_path(const char *real) {
    size_t rl = strlen(root);
    if (rl == 1) return real;
    return (real[rl] == '\0') ? "/" : real + rl;
}

/* get/put n'acceptent qu'un nom simple : pas de '/', pas de ".." (anti-évasion) */
static int valid_filename(const char *n) {
    return n[0] != '\0' && strchr(n, '/') == NULL &&
           strcmp(n, ".") != 0 && strcmp(n, "..") != 0;
}

static int reply_err(int fd, const char *tag, const char *msg) {
    return send_reply(fd, tag, 0, msg, strlen(msg));
}
static int reply_ok_text(int fd, const char *tag, const char *msg) {
    return send_reply(fd, tag, 1, msg, strlen(msg));
}

/* ---- GET_REQUEST ---- */
static int handle_get(int fd, const char *name) {
    struct stat st;
    if (!valid_filename(name)) return reply_err(fd, "GET_REPLY", "Nom de fichier invalide");
    if (stat(name, &st) < 0 || !S_ISREG(st.st_mode))
        return reply_err(fd, "GET_REPLY", "Fichier introuvable");
    FILE *f = fopen(name, "rb");                    /* "rb" : fichiers binaires (pdf, image, video) */
    if (!f) return reply_err(fd, "GET_REPLY", strerror(errno));

    char hdr[128];
    snprintf(hdr, sizeof hdr, "GET_REPLY OK %lld\n", (long long)st.st_size);
    int r = send_all(fd, hdr, strlen(hdr));
    if (r == 0) r = stream_from_file(fd, f, (long long)st.st_size);
    fclose(f);
    return r;
}

/* ---- PUT_REQUEST : args = "<nom> <taille>" ---- */
static int handle_put(int fd, char *args) {
    char *sp = strrchr(args, ' ');
    char *end;
    if (!sp) return reply_err(fd, "PUT_REPLY", "Requête PUT mal formée");
    *sp = '\0';
    long long size = strtoll(sp + 1, &end, 10);
    if (*end != '\0' || size < 0) return reply_err(fd, "PUT_REPLY", "Taille invalide");

    FILE *f = NULL;
    const char *err = NULL;
    if (!valid_filename(args)) err = "Nom de fichier invalide";
    else if (!(f = fopen(args, "wb"))) err = strerror(errno);

    /* Même en cas d'erreur, on LIT les <size> octets annoncés (f == NULL : on les jette),
     * sinon ils seraient interprétés comme des commandes. */
    int r = stream_to_file(fd, f, size);
    if (f) fclose(f);
    if (r < 0) return -1;
    if (!err && r > 0) err = "Erreur d'ecriture sur le serveur";
    if (err) return reply_err(fd, "PUT_REPLY", err);

    char msg[PATH_MAX + 64];
    snprintf(msg, sizeof msg, "Fichier '%s' ecrit sur le serveur (%lld octets)", args, size);
    return reply_ok_text(fd, "PUT_REPLY", msg);
}

/* ---- CD_REQUEST ---- */
static int handle_cd(int fd, const char *arg) {
    char old[PATH_MAX], cur[PATH_MAX];
    if (arg[0] == '\0') return reply_err(fd, "CD_REPLY", "Usage : cd <repertoire>");
    if (!getcwd(old, sizeof old)) return reply_err(fd, "CD_REPLY", strerror(errno));
    if (chdir(arg) < 0) return reply_err(fd, "CD_REPLY", strerror(errno));
    if (!getcwd(cur, sizeof cur) || !in_root(cur)) {      /* ex : cd .. depuis la racine */
        if (chdir(old) < 0) { /* ne devrait pas arriver */ }
        return reply_err(fd, "CD_REPLY", "Acces refuse : en dehors de la racine du serveur");
    }
    char msg[PATH_MAX + 32];
    snprintf(msg, sizeof msg, "Repertoire courant : %s", virtual_path(cur));
    return reply_ok_text(fd, "CD_REPLY", msg);
}

/* ---- PWD_REQUEST ---- */
static int handle_pwd(int fd) {
    char cur[PATH_MAX];
    if (!getcwd(cur, sizeof cur)) return reply_err(fd, "PWD_REPLY", strerror(errno));
    return reply_ok_text(fd, "PWD_REPLY", virtual_path(cur));
}

/* ---- LS_REQUEST [repertoire] ---- */
static int handle_ls(int fd, const char *arg) {
    char target[PATH_MAX];
    struct dirent **list;
    struct stat st;

    if (!realpath(arg[0] ? arg : ".", target)) return reply_err(fd, "LS_REPLY", strerror(errno));
    if (!in_root(target)) return reply_err(fd, "LS_REPLY", "Acces refuse : en dehors de la racine du serveur");
    int n = scandir(target, &list, NULL, alphasort);
    if (n < 0) return reply_err(fd, "LS_REPLY", strerror(errno));

    size_t total = 1;
    for (int i = 0; i < n; i++) total += strlen(list[i]->d_name) + 2;
    char *out = malloc(total);
    if (!out) { for (int i = 0; i < n; i++) free(list[i]); free(list); return reply_err(fd, "LS_REPLY", "Memoire insuffisante"); }
    out[0] = '\0';

    for (int i = 0; i < n; i++) {
        const char *nm = list[i]->d_name;
        if (strcmp(nm, ".") != 0 && strcmp(nm, "..") != 0) {
            char full[PATH_MAX + 300];
            snprintf(full, sizeof full, "%s/%s", target, nm);
            strcat(out, nm);
            if (stat(full, &st) == 0 && S_ISDIR(st.st_mode)) strcat(out, "/");
            strcat(out, "\n");
        }
        free(list[i]);
    }
    free(list);

    int r = (out[0] == '\0') ? reply_ok_text(fd, "LS_REPLY", "(vide)\n")
                             : reply_ok_text(fd, "LS_REPLY", out);
    free(out);
    return r;
}

/* ---- dialogue avec un client ---- */
static void session(int fd) {
    char line[LINE_MAX_LEN];
    while (recv_line(fd, line, sizeof line) >= 0) {
        char *arg = strchr(line, ' ');
        if (arg) *arg++ = '\0'; else arg = line + strlen(line);   /* arg = "" */
        printf("[%d] %s %s\n", getpid(), line, arg); fflush(stdout);

        int r;
        if      (!strcmp(line, "GET_REQUEST")) r = handle_get(fd, arg);
        else if (!strcmp(line, "PUT_REQUEST")) r = handle_put(fd, arg);
        else if (!strcmp(line, "CD_REQUEST"))  r = handle_cd(fd, arg);
        else if (!strcmp(line, "LS_REQUEST"))  r = handle_ls(fd, arg);
        else if (!strcmp(line, "PWD_REQUEST")) r = handle_pwd(fd);
        else r = reply_err(fd, "ERR_REPLY", "Commande inconnue");
        if (r < 0) break;                       /* erreur réseau : on abandonne ce client */
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) { fprintf(stderr, "Usage : %s <port> <racine>\n", argv[0]); return 1; }
    int port = atoi(argv[1]);
    if (port < 1 || port > 65535) { fprintf(stderr, "port invalide\n"); return 1; }
    if (chdir(argv[2]) < 0) { perror("racine"); return 1; }
    if (!getcwd(root, sizeof root)) { perror("getcwd"); return 1; }

    signal(SIGCHLD, SIG_IGN);       /* pas de zombies */
    signal(SIGPIPE, SIG_IGN);       /* client parti : send() renvoie une erreur au lieu de tuer le processus */

    int s_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ecoute == -1) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(s_ecoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

    struct sockaddr_in addr, addr_client;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s_ecoute, (struct sockaddr *)&addr, sizeof addr) == -1) { perror("bind"); return 1; }
    if (listen(s_ecoute, 5) == -1) { perror("listen"); return 1; }
    printf("Serveur FTP : port %d, racine %s\n", port, root); fflush(stdout);

    while (1) {
        socklen_t lg = sizeof addr_client;
        int s_service = accept(s_ecoute, (struct sockaddr *)&addr_client, &lg);
        if (s_service == -1) { perror("accept"); continue; }
        printf("Client connecte : %s:%d\n", inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
        fflush(stdout);
        if (fork() == 0) {
            close(s_ecoute);
            session(s_service);
            exit(0);
        }
        close(s_service);
    }
}
