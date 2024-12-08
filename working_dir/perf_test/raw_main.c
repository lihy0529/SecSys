#include <stdio.h>
#include <time.h>
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
int main() {
    
    int sizes[] = {32, 64, 128, 256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        
        clock_t start = clock();
        int result = calculate(size);
        clock_t end = clock();
        
        double duration = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Size %d: %f seconds\n", size, duration);
    }
    
    return 0;
}
