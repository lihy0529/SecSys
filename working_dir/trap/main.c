#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trap.h"
#include "wasm-rt.h"

// 当前活动的实例指针
static struct w2c_trap* active_instance;

// WASI functions implementation
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

// 定义WASI函数类型
typedef u32 (*wasi_fd_write_t)(struct w2c_wasi__unstable*, u32, u32, u32, u32);
typedef void (*wasi_proc_exit_t)(struct w2c_wasi__unstable*, u32);

// 定义WASI结构体
struct w2c_wasi__unstable {
    wasi_fd_write_t fd_write;
    wasi_proc_exit_t proc_exit;
};

int main(int argc, char* argv[]) {
    // 初始化 WebAssembly 运行时
    wasm_rt_init();
    
    // 创建实例
    struct w2c_trap instance;
    active_instance = &instance;
    
    // 创建并初始化WASI结构体
    struct w2c_wasi__unstable wasi;
    wasi.fd_write = w2c_wasi__unstable_fd_write;
    wasi.proc_exit = w2c_wasi__unstable_proc_exit;
    
    // 实例化 WebAssembly 模块
    wasm2c_trap_instantiate(&instance, &wasi);
    
    // 调用入口函数
    w2c_trap_0x5Fstart(&instance);
    
    // 清理资源
    wasm2c_trap_free(&instance);
    wasm_rt_free();
    return 0;
}