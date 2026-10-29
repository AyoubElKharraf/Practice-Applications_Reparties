/* TP2 - Mini chat en mode datagramme (UDP) : CLIENT
 *
 * Usage : ./client_chat <machine_serveur> <port>
 *
 * Le client doit faire DEUX choses en même temps :
 *   - lire le clavier et envoyer les messages au serveur (sendto) ;
 *   - recevoir et afficher les messages des autres (recvfrom).
 * Ces deux opérations sont bloquantes : on utilise donc deux processus (fork),
 * qui partagent la même socket :
 *   - le PÈRE lit le clavier et envoie ;
 *   - le FILS reçoit et affiche.
 *
 * Pour quitter : "/quit", Ctrl+D ou Ctrl+C (le client prévient alors le serveur).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MSG_MAX   225
#define BUF_SIZE  256
#define LINE_SIZE 1024
#define CMD_QUIT  "/quit"

static volatile sig_atomic_t interrompu = 0;
static void on_sigint(int sig) { (void)sig; interrompu = 1; }

int main(int argc, char *argv[]) {
    if (argc != 3) { fprintf(stderr, "Usage : %s <machine_serveur> <port>\n", argv[0]); return 1; }
    int port = atoi(argv[2]);
    if (port < 1 || port > 65535) { fprintf(stderr, "port invalide : %s\n", argv[2]); return 1; }

    /* adresse du serveur */
    struct hostent *host = gethostbyname(argv[1]);
    if (host == NULL) { fprintf(stderr, "machine inconnue : %s\n", argv[1]); return 1; }
    struct sockaddr_in addr_serveur;
    memset(&addr_serveur, 0, sizeof addr_serveur);
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_port = htons((unsigned short)port);
    memcpy(&addr_serveur.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) { perror("socket"); return 1; }

    /* Nommage explicite avec le port 0 : le système choisit un port libre.
     * On le fait AVANT le fork pour que le père et le fils partagent la même
     * adresse locale (c'est elle que le serveur mémorise et utilise pour répondre). */
    struct sockaddr_in addr_locale;
    memset(&addr_locale, 0, sizeof addr_locale);
    addr_locale.sin_family = AF_INET;
    addr_locale.sin_port = htons(0);
    addr_locale.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&addr_locale, sizeof addr_locale) == -1) {
        perror("bind");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {
        /* ---------- FILS : réception et affichage ---------- */
        char buf[BUF_SIZE];
        while (1) {
            ssize_t n = recvfrom(s, buf, BUF_SIZE - 1, 0, NULL, NULL);
            if (n < 0) { perror("recvfrom"); break; }
            buf[n] = '\0';
            printf("%s\n", buf);
            fflush(stdout);
        }
        exit(1);
    }

    /* ---------- PÈRE : lecture du clavier et envoi ---------- */
    /* Ctrl+C : sans SA_RESTART, fgets() est interrompu et on peut quitter proprement */
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    char ligne[LINE_SIZE];
    int parti = 0;
    while (!interrompu && fgets(ligne, sizeof ligne, stdin) != NULL) {
        size_t len = strcspn(ligne, "\r\n");
        int ligne_entiere = (ligne[len] != '\0');
        ligne[len] = '\0';
        if (!ligne_entiere) {                   /* ligne plus longue que le tampon : on jette la suite */
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        }
        if (len == 0) continue;                 /* ligne vide : rien à envoyer */

        if (len > MSG_MAX) {                    /* trop long : on tronque */
            len = MSG_MAX;
            while (len > 0 && ((unsigned char)ligne[len] & 0xC0) == 0x80)
                len--;                          /* sans couper un caractère UTF-8 en deux */
            ligne[len] = '\0';
            fprintf(stderr, "(message tronqué à %d caractères)\n", MSG_MAX);
        }

        if (sendto(s, ligne, len, 0, (struct sockaddr *)&addr_serveur, sizeof addr_serveur) == -1)
            perror("sendto");
        if (strcmp(ligne, CMD_QUIT) == 0) { parti = 1; break; }
    }

    /* fin (Ctrl+D ou Ctrl+C) : on prévient le serveur si ce n'est pas déjà fait */
    if (!parti)
        sendto(s, CMD_QUIT, strlen(CMD_QUIT), 0, (struct sockaddr *)&addr_serveur, sizeof addr_serveur);

    kill(pid, SIGTERM);                         /* arrêt du fils */
    waitpid(pid, NULL, 0);
    close(s);
    return 0;
}
