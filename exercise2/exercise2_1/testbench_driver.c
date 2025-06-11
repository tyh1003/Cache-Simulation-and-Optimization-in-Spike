#define I 1000
#define K 1000

#include <stdio.h>
#include <string.h>

#define GOLDEN_RATIO_32 0x61C88647
static inline int hash_32(int val) {
    return (val * GOLDEN_RATIO_32) >> 22 & 0x3FF;
}

extern void matrix_transpose(int n, int (*dst)[K], int (*src)[K]);

int main(int argc, char *argv[]) {
    int input[I][K] __attribute__((aligned(4096)));
    int output[I][K] __attribute__((aligned(4096)));

    int seed = 0;

    memset(output, 0, K * I * sizeof(int));
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < K; j++)
            input[i][j] = hash_32(i * K + j);
    }


    matrix_transpose(I, output, input);

    if (argc == 2) {
        FILE *f = fopen(argv[1], "w");
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < I; j++) {
                fprintf(f, "%d ", output[i][j]);
            }
            fprintf(f, "\n");
        }
        fclose(f);
    }

    return 0;
}