#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "wasm-rt.h"
#include "vulnerable.h"
// WASM 运行时环境结构
struct w2c_env {
    struct {
        uint32_t size;
        uint8_t *data;
    } memory;
};
static int g_argc;
static char** g_argv;

static struct w2c_vulnerable* active_instance;

// 添加 WASI 结构体定义
struct w2c_wasi__unstable {
    // WASI 实例不需要存储任何状态
    int dummy;  // 添加一个虚拟字段以避免空结构体
};

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
    if (argv_ptr >= active_instance->w2c_memory.size || 
        argv_buf_ptr >= active_instance->w2c_memory.size) {
        return -1;
    }
    
    u32* argv = (u32*)&active_instance->w2c_memory.data[argv_ptr];
    char* argv_buf = (char*)&active_instance->w2c_memory.data[argv_buf_ptr];
    
    u32 current_buf_ptr = argv_buf_ptr;
    for (int i = 0; i < g_argc; i++) {
        if (current_buf_ptr + strlen(g_argv[i]) + 1 > active_instance->w2c_memory.size) {
            return -1;
        }
        argv[i] = current_buf_ptr;
        strncpy(argv_buf, g_argv[i], active_instance->w2c_memory.size - current_buf_ptr);
        current_buf_ptr += strlen(g_argv[i]) + 1;
        argv_buf += strlen(g_argv[i]) + 1;
    }
    return 0;
}

u32 w2c_wasi__unstable_fd_write(struct w2c_wasi__unstable* wasi, u32 fd, u32 iovs_ptr, u32 iovs_len, u32 nwritten_ptr) {
    if (iovs_ptr >= active_instance->w2c_memory.size || 
        nwritten_ptr >= active_instance->w2c_memory.size) {
        return -1;
    }
    
    struct iovec {
        u32 buf_ptr;
        u32 buf_len;
    } *iovs = (struct iovec*)&active_instance->w2c_memory.data[iovs_ptr];
    
    size_t total_written = 0;
    for (u32 i = 0; i < iovs_len; i++) {
        if (iovs[i].buf_ptr >= active_instance->w2c_memory.size ||
            iovs[i].buf_ptr + iovs[i].buf_len > active_instance->w2c_memory.size) {
            return -1;
        }
        char* buf = (char*)&active_instance->w2c_memory.data[iovs[i].buf_ptr];
        ssize_t written = write(fd, buf, iovs[i].buf_len);
        if (written < 0) return -1;
        total_written += written;
    }
    
    u32* nwritten = (u32*)&active_instance->w2c_memory.data[nwritten_ptr];
    *nwritten = total_written;
    return 0;
}

void w2c_wasi__unstable_proc_exit(struct w2c_wasi__unstable* wasi, u32 code) {
    printf("\nProgram exiting with code: %d\n", code);
    
    // exit(code);  // 确保程序真正退出
}

// 辅助函数：打印内存内容
void print_memory(uint8_t *data, size_t offset, size_t length) {
    printf("Memory at offset %zu: ", offset);
    for (size_t i = 0; i < length; i++) {
        printf("%02x ", data[offset + i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    // 保存命令行参数
    g_argc = argc;
    g_argv = argv;

    // 初始化WASM实例
    struct w2c_vulnerable instance;
    struct w2c_wasi__unstable wasi = {0};
    
    wasm_rt_init();
    wasm2c_vulnerable_instantiate(&instance, &wasi);
    active_instance = &instance;

    // 初始化测试数据到WASM内存
    const char *safe_str = "SAFE";
    const char *unsafe_str = "UNSAFE";
    memcpy(instance.w2c_memory.data + 0, safe_str, strlen(safe_str) + 1);
    memcpy(instance.w2c_memory.data + 8, unsafe_str, strlen(unsafe_str) + 1);

    printf("Initial memory state:\n");
    print_memory(instance.w2c_memory.data, 0, 32);
    print_memory(instance.w2c_memory.data, 2048, 32);

    // 运行 WASM 程序
    printf("\nRunning WASM program...\n");
    w2c_vulnerable_0x5Fstart(&instance);

    printf("\nWARNING: Program did not exit properly!\n");

    printf("\nFinal memory state:\n");
    print_memory(instance.w2c_memory.data, 0, 32);
    print_memory(instance.w2c_memory.data, 2048, 40);

    // 清理
    wasm2c_vulnerable_free(&instance);
    wasm_rt_free();
    
    return 0;
}
