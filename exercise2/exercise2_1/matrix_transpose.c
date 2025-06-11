void matrix_transpose(int n, int *dst, int *src) {
        for (int x = 0; x < n; x++) {
            for (int y = 0; y < n; y++) {
                dst[y + x * n] = src[x + y * n];
            }
        }
    }