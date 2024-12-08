#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "echo.h"
#include "wasm-rt.h"

// Global variables to store argc and argv
static int g_argc;
static char** g_argv;

// 当前活动的实例指针
static struct w2c_echo* active_instance;

// WASI functions implementation
u32 w2c_wasi__unstable_args_sizes_get(struct w2c_wasi__unstable* wasi, u32 argc_ptr, u32 argv_buf_size_ptr) {
    u32* argc = (u32*)&active_instance->w2c_memory.data[argc_ptr];
    u32* argv_buf_size = (u32*)&active_instance->w2c_memory.data[argv_buf_size_ptr];
    
    *argc = g_argc;
    size_t total_size = 0;
    for (int i = 0; i < g_argc; i++) {
        total_size += strlen(g_argv[i]) + 1;
    }
    *argv_buf_size = total_size;
    return 0;
}

u32 w2c_wasi__unstable_args_get(struct w2c_wasi__unstable* wasi, u32 argv_ptr, u32 argv_buf_ptr) {
    u32* argv = (u32*)&active_instance->w2c_memory.data[argv_ptr];
    char* argv_buf = (char*)&active_instance->w2c_memory.data[argv_buf_ptr];
    
    u32 current_buf_ptr = argv_buf_ptr;
    for (int i = 0; i < g_argc; i++) {
        argv[i] = current_buf_ptr;
        strcpy(argv_buf, g_argv[i]);
        current_buf_ptr += strlen(g_argv[i]) + 1;
        argv_buf += strlen(g_argv[i]) + 1;
    }
    return 0;
}

u32 w2c_wasi__unstable_fd_write(struct w2c_wasi__unstable* wasi, u32 fd, u32 iovs_ptr, u32 iovs_len, u32 nwritten_ptr) {
    struct iovec {
        u32 buf_ptr;
        u32 buf_len;
    } *iovs = (struct iovec*)&active_instance->w2c_memory.data[iovs_ptr];
    
    size_t total_written = 0;
    for (u32 i = 0; i < iovs_len; i++) {
        char* buf = (char*)&active_instance->w2c_memory.data[iovs[i].buf_ptr];
        total_written += write(fd, buf, iovs[i].buf_len);
    }
    
    u32* nwritten = (u32*)&active_instance->w2c_memory.data[nwritten_ptr];
    *nwritten = total_written;
    return 0;
}

void w2c_wasi__unstable_proc_exit(struct w2c_wasi__unstable* wasi, u32 code) {
    exit(code);
}

int main(int argc, char* argv[]) {
    g_argc = argc;
    g_argv = argv;
    
    wasm_rt_init();
    
    // Create instance
    struct w2c_echo instance;
    active_instance = &instance;  // 设置活动实例
    wasm2c_echo_instantiate(&instance, NULL);
    
    w2c_echo_0x5Fstart(&instance);
    
    wasm2c_echo_free(&instance);
    wasm_rt_free();
    return 0;
}