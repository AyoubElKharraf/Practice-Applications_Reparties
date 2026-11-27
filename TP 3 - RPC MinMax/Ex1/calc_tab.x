/* calc_tab.x */
typedef double tab_t<>;

struct minmax_res {
    double min_val;
    double max_val;
};

program CALC_TAB_PROG {
    version CALC_TAB_VERS {
        minmax_res MINMAX(tab_t) = 1;
        double SOM(tab_t) = 2;
    } = 1;
} = 0x20000101;