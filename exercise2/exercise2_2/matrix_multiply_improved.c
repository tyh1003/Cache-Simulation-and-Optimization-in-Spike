#define BLOCK_SIZE 8
void matrix_multiply(int *a, int *b, int *output, int i,int k, int j) {
    // Inplement your code here
    for (int i0 = 0; i0 < i; i0 += BLOCK_SIZE) {
        for (int j0 = 0; j0 < j; j0 += BLOCK_SIZE) {
            for (int k0 = 0; k0 < k; k0 += BLOCK_SIZE) {
                // block 區域
                for (int ii = i0; ii < i0 + BLOCK_SIZE && ii < i; ii++) {
                    for (int jj = j0; jj < j0 + BLOCK_SIZE && jj < j; jj++) {
                        int sum = output[ii * j + jj]; // 可設 0，若 output 先歸零
                        for (int kk = k0; kk < k0 + BLOCK_SIZE && kk < k; kk++) {
                            sum += a[ii * k + kk] * b[kk * j + jj];
                        }
                        output[ii * j + jj] = sum;
                    }
                }
            }
        }
    }
}