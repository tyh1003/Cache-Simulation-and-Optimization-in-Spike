void matrix_multiply(int *A, int *B, int *Output, int i, int k, int j) {
    for (int x = 0; x < i; x++) {
        for (int y = 0; y < j; y++) {
            int sum = 0;
            for (int z = 0; z < k; z++) {
                sum += A[x * k + z] * B[z * j + y];
            }
            Output[x * j + y] = sum;
        }
    }
}