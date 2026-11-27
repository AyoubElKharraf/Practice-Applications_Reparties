/* calc_tab_server.c */
#include <stdio.h>
#include <stdlib.h>
#include "calc_tab.h"

minmax_res *
minmax_1_svc(tab_t *argp, struct svc_req *rqstp)
{
    static minmax_res res;
    u_int n = argp->tab_t_len;
    double *data = argp->tab_t_val;

    if (n == 0 || data == NULL) {
        res.min_val = 0.0;
        res.max_val = 0.0;
        return &res;
    }

    res.min_val = data[0];
    res.max_val = data[0];

    for (u_int i = 1; i < n; i++) {
        if (data[i] < res.min_val) res.min_val = data[i];
        if (data[i] > res.max_val) res.max_val = data[i];
    }

    printf("[Serveur MINMAX] %u elements traites -> min=%.2f, max=%.2f\n",
           n, res.min_val, res.max_val);
    return &res;
}

double *
som_1_svc(tab_t *argp, struct svc_req *rqstp)
{
    static double somme;
    u_int n = argp->tab_t_len;
    double *data = argp->tab_t_val;

    somme = 0.0;
    for (u_int i = 0; i < n; i++) {
        somme += data[i];
    }

    printf("[Serveur SOM] %u elements traites -> somme partielle=%.2f\n", n, somme);
    return &somme;
}