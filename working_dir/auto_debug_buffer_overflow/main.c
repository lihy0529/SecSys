#include <stdio.h>
#include "auto_debug.h"

int main(int argc, char** argv) {
    int i = atoi(argv[1]);
    wasm_rt_init();
    
    // Create an instance
    w2c_auto__debug instance;
    
    // Initialize the WASM module
    wasm2c_auto__debug_instantiate(&instance);
    
    // Initialize emscripten stack
    w2c_auto__debug_emscripten_stack_init(&instance);
    w2c_auto__debug_0x5Finitialize(&instance);
    
    // Call the calculate function (assuming this is what you want to test)
    // You can modify the input parameter (10 in this case) as needed
    u32 result = w2c_auto__debug_calculate(&instance, i);
    printf("Result: %d\n", result);
    // Clean up
    wasm2c_auto__debug_free(&instance);
    
    return 0;
}
