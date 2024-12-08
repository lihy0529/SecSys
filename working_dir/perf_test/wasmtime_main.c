#include <stdio.h>
#include <time.h>
#include "wasm.h"
#include "wasmtime.h"
#include "wasmtime_perf_test.h"

int main() {
    // 初始化 WASM 运行时
    wasm_rt_init();
    w2c_wasmtime_perf_test_instantiate();
    
    int sizes[] = {32, 64, 128, 256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        
        clock_t start = clock();
        int result = w2c_wasmtime_perf_test_calculate(size);
        clock_t end = clock();
        
        double duration = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Result: %d\n", result);
        printf("Size %d: %f seconds\n", size, duration);
    }
    
    return 0;
}
