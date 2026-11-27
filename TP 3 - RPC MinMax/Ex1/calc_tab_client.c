/* calc_tab_client.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "calc_tab.h"

typedef struct {
    int id;
    const char *host;
    size_t offset;
    size_t chunk_len;
    double *tab;
    double somme_partielle;
    double min_partiel;
    double max_partiel;
    int has_minmax;
    int status; /* 0 = OK, -1 = erreur */
} tab_worker_arg_t;

static void *
tab_worker(void *raw_arg)
{
    tab_worker_arg_t *w = (tab_worker_arg_t *)raw_arg;
    w->status = -1;
    w->somme_partielle = 0.0;
    w->has_minmax = 0;

    CLIENT *clnt = clnt_create((char *)w->host, CALC_TAB_PROG, CALC_TAB_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(w->host);
        return NULL;
    }

    tab_t sous_tab;
    sous_tab.tab_t_len = (u_int)w->chunk_len;
    sous_tab.tab_t_val = w->tab + w->offset;

    double *res_som = som_1(&sous_tab, clnt);
    if (res_som == NULL) {
        clnt_perror(clnt, "Erreur appel SOM");
        clnt_destroy(clnt);
        return NULL;
    }
    w->somme_partielle = *res_som;

    minmax_res *res_mm = minmax_1(&sous_tab, clnt);
    if (res_mm != NULL) {
        w->min_partiel = res_mm->min_val;
        w->max_partiel = res_mm->max_val;
        w->has_minmax = 1;
    }

    clnt_destroy(clnt);
    w->status = 0;
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <taille_tableau> <hote1> [hote2 ...]\n", argv[0]);
        return 1;
    }

    size_t N = (size_t)atol(argv[1]);
    int max_threads = argc - 2;
    char **hosts = &argv[2];

    if (N == 0) {
        fprintf(stderr, "Taille du tableau invalide.\n");
        return 1;
    }

    double *tab = (double *)malloc(N * sizeof(double));
    if (!tab) {
        perror("malloc");
        return 1;
    }
    for (size_t i = 0; i < N; i++) {
        tab[i] = (double)(i + 1);
    }

    int nb_workers = ((size_t)max_threads < N) ? max_threads : (int)N;
    pthread_t *threads = (pthread_t *)calloc((size_t)nb_workers, sizeof(pthread_t));
    tab_worker_arg_t *args = (tab_worker_arg_t *)calloc((size_t)nb_workers, sizeof(tab_worker_arg_t));

    if (!threads || !args) {
        perror("calloc");
        free(threads);
        free(args);
        free(tab);
        return 1;
    }

    size_t base_chunk = N / (size_t)nb_workers;
    size_t reste = N % (size_t)nb_workers;
    size_t offset = 0;
    int launched = 0;

    for (int s = 0; s < nb_workers; s++) {
        size_t chunk_len = base_chunk + ((size_t)s < reste ? 1 : 0);
        args[s].id = s;
        args[s].host = hosts[s];
        args[s].offset = offset;
        args[s].chunk_len = chunk_len;
        args[s].tab = tab;

        if (pthread_create(&threads[s], NULL, tab_worker, &args[s]) != 0) {
            perror("pthread_create");
            break;
        }
        launched++;
        offset += chunk_len;
    }

    double somme_globale = 0.0;
    double min_global = 0.0;
    double max_global = 0.0;
    int mm_init = 0;
    int err = 0;

    for (int s = 0; s < launched; s++) {
        pthread_join(threads[s], NULL);
        if (args[s].status != 0) {
            err = 1;
            continue;
        }
        printf("[Thread %d - %s] [%zu..%zu] -> somme = %.2f",
               args[s].id, args[s].host, args[s].offset,
               args[s].offset + args[s].chunk_len - 1, args[s].somme_partielle);
        if (args[s].has_minmax) {
            printf(" (min=%.2f, max=%.2f)", args[s].min_partiel, args[s].max_partiel);
            if (!mm_init || args[s].min_partiel < min_global) min_global = args[s].min_partiel;
            if (!mm_init || args[s].max_partiel > max_global) max_global = args[s].max_partiel;
            mm_init = 1;
        }
        printf("\n");
        somme_globale += args[s].somme_partielle;
    }

    if (!err) {
        printf("\n==> Somme globale (N=%zu) = %.2f (attendu: %.2f)\n",
               N, somme_globale, (double)N * ((double)N + 1.0) / 2.0);
        if (mm_init) {
            printf("==> Min global = %.2f | Max global = %.2f\n", min_global, max_global);
        }
    }

    free(threads);
    free(args);
    free(tab);
    return err ? 1 : 0;
}