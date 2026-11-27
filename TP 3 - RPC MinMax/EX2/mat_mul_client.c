/* mat_mul_client.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mat_mul.h"

typedef struct {
    int id;
    const char *host;
    int row_offset;
    int slice_rows;
    int K;
    int N;
    double *A;
    double *B;
    double *C;
    int status; /* 0 = OK, -1 = erreur */
} mat_worker_arg_t;

static void *
mat_worker(void *raw_arg)
{
    mat_worker_arg_t *w = (mat_worker_arg_t *)raw_arg;
    w->status = -1;

    CLIENT *clnt = clnt_create((char *)w->host, MAT_MUL_PROG, MAT_MUL_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(w->host);
        return NULL;
    }

    mul_task_args args;
    args.A_slice.rows = w->slice_rows;
    args.A_slice.cols = w->K;
    args.A_slice.data.mat_elem_t_len = (u_int)((size_t)w->slice_rows * (size_t)w->K);
    args.A_slice.data.mat_elem_t_val = w->A + (size_t)w->row_offset * (size_t)w->K;

    args.B.rows = w->K;
    args.B.cols = w->N;
    args.B.data.mat_elem_t_len = (u_int)((size_t)w->K * (size_t)w->N);
    args.B.data.mat_elem_t_val = w->B;

    matrix_t *part = mul_block_1(&args, clnt);
    if (part == NULL || part->rows != w->slice_rows || part->cols != w->N) {
        clnt_perror(clnt, "Erreur MUL_BLOCK");
        clnt_destroy(clnt);
        return NULL;
    }

    memcpy(w->C + (size_t)w->row_offset * (size_t)w->N,
           part->data.mat_elem_t_val,
           (size_t)w->slice_rows * (size_t)w->N * sizeof(double));

    clnt_destroy(clnt);
    w->status = 0;
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <M> <K> <N> <hote1> [hote2 ...]\n", argv[0]);
        fprintf(stderr, "Calcule C(M x N) = A(M x K) * B(K x N)\n");
        return 1;
    }

    int M = atoi(argv[1]);
    int K = atoi(argv[2]);
    int N = atoi(argv[3]);
    int max_threads = argc - 4;
    char **hosts = &argv[4];

    if (M <= 0 || K <= 0 || N <= 0) {
        fprintf(stderr, "Dimensions invalides.\n");
        return 1;
    }

    double *A = (double *)malloc((size_t)M * (size_t)K * sizeof(double));
    double *B = (double *)malloc((size_t)K * (size_t)N * sizeof(double));
    double *C = (double *)calloc((size_t)M * (size_t)N, sizeof(double));

    if (!A || !B || !C) {
        perror("malloc/calloc");
        free(A); free(B); free(C);
        return 1;
    }

    for (int i = 0; i < M; i++)
        for (int j = 0; j < K; j++)
            A[i * K + j] = (double)(i + 1);

    for (int i = 0; i < K; i++)
        for (int j = 0; j < N; j++)
            B[i * N + j] = (double)(j + 1);

    int nb_workers = (max_threads < M) ? max_threads : M;
    pthread_t *threads = (pthread_t *)calloc((size_t)nb_workers, sizeof(pthread_t));
    mat_worker_arg_t *args = (mat_worker_arg_t *)calloc((size_t)nb_workers, sizeof(mat_worker_arg_t));

    if (!threads || !args) {
        perror("calloc");
        free(threads); free(args); free(A); free(B); free(C);
        return 1;
    }

    int base_rows = M / nb_workers;
    int rem_rows = M % nb_workers;
    int row_offset = 0;
    int launched = 0;

    for (int s = 0; s < nb_workers; s++) {
        int slice_rows = base_rows + (s < rem_rows ? 1 : 0);
        args[s].id = s;
        args[s].host = hosts[s];
        args[s].row_offset = row_offset;
        args[s].slice_rows = slice_rows;
        args[s].K = K;
        args[s].N = N;
        args[s].A = A;
        args[s].B = B;
        args[s].C = C;

        if (pthread_create(&threads[s], NULL, mat_worker, &args[s]) != 0) {
            perror("pthread_create");
            break;
        }
        launched++;
        row_offset += slice_rows;
    }

    int err = 0;
    for (int s = 0; s < launched; s++) {
        pthread_join(threads[s], NULL);
        if (args[s].status != 0) {
            err = 1;
        } else {
            printf("[Thread %d - %s] lignes [%d..%d] calculees.\n",
                   args[s].id, args[s].host,
                   args[s].row_offset, args[s].row_offset + args[s].slice_rows - 1);
        }
    }

    if (!err) {
        if (M <= 8 && N <= 8) {
            printf("\nMatrice resultat C (%d x %d) :\n", M, N);
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    printf("%8.1f ", C[i * N + j]);
                }
                printf("\n");
            }
        } else {
            printf("\nProduit (%d x %d) * (%d x %d) -> C(%d x %d) termine. C[0][0] = %.1f\n",
                   M, K, K, N, M, N, C[0]);
        }
    }

    free(threads);
    free(args);
    free(A);
    free(B);
    free(C);
    return err ? 1 : 0;
}