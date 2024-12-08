#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int mat_a[2048][2048] = {};
int mat_b[2048][2048] = {};
int mat_c[2048][2048] = {};
int calculate(int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                mat_c[i][j] += mat_a[i][k] * mat_b[k][j];
            }
        }
    }
    return mat_c[0][0];
}