#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hello.h"
#include "wasm-rt.h"

// 当前活动的实例指针
static struct w2c_hello* active_instance;

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

int main(int argc, char* argv[]) {
    // 初始化 WebAssembly 运行时
    wasm_rt_init();
    
    // 创建实例
    struct w2c_hello instance;
    active_instance = &instance;  // 设置活动实例
    
    // 实例化 WebAssembly 模块
    wasm2c_hello_instantiate(&instance, NULL);
    
    // 调用入口函数
    w2c_hello_0x5Fstart(&instance);
    
    // 清理资源
    wasm2c_hello_free(&instance);
    wasm_rt_free();
    return 0;
}
