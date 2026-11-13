#include "calcule.h"

int *puiss2_1_svc(int *argp, struct svc_req *rqstp) {
    static int result;
    int i;
    int n = *argp;

    /* Cas 2^0 = 1 */
    if (n <= 0) {
        result = 1;
        return &result;
    }

    result = 2;
    for (i = 1; i < n; i++) {
        result = result * 2;
    }
    return &result;
}

float *multxy_1_svc(operandes *argp, struct svc_req *rqstp) {
    static float result;
    result = argp->x * argp->y;
    return &result;
}
