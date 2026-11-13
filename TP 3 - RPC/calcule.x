struct operandes {
    float x;
    float y;
};

program MATHS_PROG {
    version MATHS_VERSION_1 {
        int PUISS2(int) = 1;
        float MULTXY(operandes) = 2;
    } = 1;
} = 0x20000002;
