#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matrix.h"

// Define the environment structure
struct w2c_env {
    wasm_rt_memory_t memory;
};

// Define environment functions
wasm_rt_memory_t* w2c_env_memory(struct w2c_env* env) {
    return &env->memory;
}

u32 w2c_env_rand(struct w2c_env* env) {
    return rand();
}

int main() {
    // Initialize runtime
    wasm_rt_init();
    srand(time(NULL));

    // Create and initialize environment
    struct w2c_env env = {0};
    env.memory.size = 1024 * 1024 * 1024;  // 4MB
    env.memory.data = calloc(env.memory.size, 1);

    // Create and initialize instance
    w2c_matrix instance;
    wasm2c_matrix_instantiate(&instance, &env);
    
    // Run the performance test
    w2c_matrix_performance_test(&instance);

    // Cleanup
    free(env.memory.data);
    wasm_rt_free();
    return 0;
}