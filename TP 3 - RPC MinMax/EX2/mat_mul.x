/* mat_mul.x */
typedef double mat_elem_t<>;

struct matrix_t {
    int rows;
    int cols;
    mat_elem_t data;
};

struct mul_task_args {
    matrix_t A_slice; /* Bloc de lignes de A : (r_slice x K) */
    matrix_t B;       /* Matrice B complète : (K x N) */
};

program MAT_MUL_PROG {
    version MAT_MUL_VERS {
        matrix_t MUL_BLOCK(mul_task_args) = 1;
    } = 1;
} = 0x20000102;