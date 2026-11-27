/* mat_mul_server.c */
#include <stdio.h>
#include <stdlib.h>
#include "mat_mul.h"

matrix_t *
mul_block_1_svc(mul_task_args *argp, struct svc_req *rqstp)
{
    static matrix_t res;
    static double *buf = NULL;
    static size_t buf_cap = 0;

    int rA = argp->A_slice.rows;
    int cA = argp->A_slice.cols; /* == rB */
    int rB = argp->B.rows;
    int cB = argp->B.cols;

    double *A = argp->A_slice.data.mat_elem_t_val;
    double *B = argp->B.data.mat_elem_t_val;

    res.rows = rA;
    res.cols = cB;
    res.data.mat_elem_t_len = 0;
    res.data.mat_elem_t_val = NULL;

    if (rA <= 0 || cA <= 0 || cB <= 0 || cA != rB || !A || !B) {
        return &res;
    }

    size_t needed = (size_t)rA * (size_t)cB;
    if (needed > buf_cap) {
        double *new_buf = (double *)realloc(buf, needed * sizeof(double));
        if (!new_buf) return &res;
        buf = new_buf;
        buf_cap = needed;
    }

    /* Calcul C_slice[i][j] = sum_k A_slice[i][k] * B[k][j] */
    for (int i = 0; i < rA; i++) {
        for (int j = 0; j < cB; j++) {
            double acc = 0.0;
            for (int k = 0; k < cA; k++) {
                acc += A[i * cA + k] * B[k * cB + j];
            }
            buf[i * cB + j] = acc;
        }
    }

    res.data.mat_elem_t_len = (u_int)needed;
    res.data.mat_elem_t_val = buf;

    printf("[Serveur MUL] Bloc (%d x %d) * (%d x %d) -> (%d x %d) calcule.\n",
           rA, cA, rB, cB, rA, cB);
    return &res;
}