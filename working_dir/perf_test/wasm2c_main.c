#include <stdio.h>
#include "perf_test.h"
#include <time.h>

int main(int argc, char** argv) {
    wasm_rt_init();
    
    // Create an instance
    w2c_perf__test instance;
    
    // Initialize the WASM module
    wasm2c_perf__test_instantiate(&instance);
    
    // Initialize emscripten stack
    w2c_perf__test_emscripten_stack_init(&instance);
    w2c_perf__test_0x5Finitialize(&instance);
    
    // Call the calculate function (assuming this is what you want to test)
    // You can modify the input parameter (10 in this case) as needed
    clock_t start = clock();
    u32 result = w2c_perf__test_calculate(&instance, 32);
    clock_t end = clock();
    printf("Size 32: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    start = clock();
    result = w2c_perf__test_calculate(&instance, 64);
    end = clock();
    printf("Size 64: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    start = clock();
    result = w2c_perf__test_calculate(&instance, 128);
    end = clock();
    printf("Size 128: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    start = clock();
    result = w2c_perf__test_calculate(&instance, 256);
    end = clock();
    printf("Size 256: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    start = clock();
    result = w2c_perf__test_calculate(&instance, 512);
    end = clock();
    printf("Size 512: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    start = clock();
    result = w2c_perf__test_calculate(&instance, 1024);
    end = clock();
    printf("Size 1024: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
    start = clock();
    result = w2c_perf__test_calculate(&instance, 2048);
    end = clock();
    printf("Size 2048: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);

    // Clean up
    wasm2c_perf__test_free(&instance);
    
    return 0;
}
