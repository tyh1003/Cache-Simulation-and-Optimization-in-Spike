
#define I 100
#define K 100
#define J 100

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define GOLDEN_RATIO_32 0x61C88647
static inline int hash_32(int val) {
    return (val * GOLDEN_RATIO_32) >> 22 & 0x3FF;
}

extern void matrix_multiply(int (*A)[K], int (*B)[J],
                                  int (*Output)[J], int i, int k,
                                  int j);

int main(int argc, char *argv[]) {
    uint64_t start_cycles, end_cycles;

    int A[I][K] __attribute__((aligned(4096)));
    int B[K][J] __attribute__((aligned(4096)));
    int Output[I][J] __attribute__((aligned(4096)));

    int seed = 0;

    memset(Output, 0, I * J * sizeof(int));
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < K; j++)
            A[i][j] = hash_32(i * K + j);
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < J; j++)
            B[i][j] = hash_32(i * J + j);
    }
    
    matrix_multiply(A, B, Output, I, K, J);

    if (argc == 2) {
        FILE *f = fopen(argv[1], "w");
        for (int i = 0; i < I; i++) {
            for (int j = 0; j < J; j++) {
                fprintf(f, "%d ", Output[i][j]);
            }
            fprintf(f, "\n");
        }
        fclose(f);
    }

    return 0;
}
