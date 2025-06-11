#define BLOCK_SIZE 8
void matrix_transpose(int n, int *dst, int *src) {
    // Inplement your code here
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            // 處理一個 block
            for (int x = i; x < i + BLOCK_SIZE && x < n; x++) {
                for (int y = j; y < j + BLOCK_SIZE && y < n; y++) {
                    dst[y + x * n] = src[x + y * n];
                }
            }
        }
    }
}