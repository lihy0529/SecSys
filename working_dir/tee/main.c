#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "tee.h"
#include "wasm-rt.h"

// 当前活动的实例指针
static struct w2c_tee* active_instance;

// Global variables to store argc and argv
static int g_argc;
static char** g_argv;

// 定义WASI函数类型
typedef u32 (*wasi_args_sizes_get_t)(struct w2c_wasi__unstable*, u32, u32);
typedef u32 (*wasi_args_get_t)(struct w2c_wasi__unstable*, u32, u32);
typedef u32 (*wasi_fd_write_t)(struct w2c_wasi__unstable*, u32, u32, u32, u32);
typedef u32 (*wasi_fd_read_t)(struct w2c_wasi__unstable*, u32, u32, u32, u32);
typedef u32 (*wasi_path_open_t)(struct w2c_wasi__unstable*, u32, u32, u32, u32, u32, u64, u64, u32, u32);
typedef void (*wasi_proc_exit_t)(struct w2c_wasi__unstable*, u32);

// 定义WASI结构体
struct w2c_wasi__unstable {
    wasi_args_sizes_get_t args_sizes_get;
    wasi_args_get_t args_get;
    wasi_fd_write_t fd_write;
    wasi_fd_read_t fd_read;
    wasi_path_open_t path_open;
    wasi_proc_exit_t proc_exit;
};

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

u32 w2c_wasi__unstable_fd_read(struct w2c_wasi__unstable* wasi, u32 fd, u32 iovs_ptr, u32 iovs_len, u32 nread_ptr) {
    struct iovec {
        u32 buf_ptr;
        u32 buf_len;
    } *iovs = (struct iovec*)&active_instance->w2c_memory.data[iovs_ptr];
    
    size_t total_read = 0;
    for (u32 i = 0; i < iovs_len; i++) {
        char* buf = (char*)&active_instance->w2c_memory.data[iovs[i].buf_ptr];
        ssize_t bytes_read = read(fd, buf, iovs[i].buf_len);
        if (bytes_read < 0) return 1;  // Error
        total_read += bytes_read;
        if (bytes_read < iovs[i].buf_len) break;  // EOF or partial read
    }
    
    u32* nread = (u32*)&active_instance->w2c_memory.data[nread_ptr];
    *nread = total_read;
    return 0;
}

u32 w2c_wasi__unstable_path_open(struct w2c_wasi__unstable* wasi, u32 dirfd, u32 dirflags, 
                                u32 path_ptr, u32 path_len, u32 oflags, u64 fs_rights_base,
                                u64 fs_rights_inheriting, u32 fs_flags, u32 fd_ptr) {
    char* path = (char*)&active_instance->w2c_memory.data[path_ptr];
    char path_buf[path_len + 1];
    memcpy(path_buf, path, path_len);
    path_buf[path_len] = '\0';
    
    int flags = O_CREAT | O_WRONLY;
    if (oflags & 0x1) flags |= O_TRUNC;  // CREAT
    if (oflags & 0x2) flags |= O_APPEND;  // APPEND
    
    int fd = open(path_buf, flags, 0644);
    if (fd < 0) return 1;  // Error
    
    u32* fd_out = (u32*)&active_instance->w2c_memory.data[fd_ptr];
    *fd_out = fd;
    return 0;
}

void w2c_wasi__unstable_proc_exit(struct w2c_wasi__unstable* wasi, u32 code) {
    exit(code);
}

int main(int argc, char* argv[]) {
    g_argc = argc;
    g_argv = argv;
    
    wasm_rt_init();
    
    struct w2c_tee instance;
    active_instance = &instance;
    
    // 创建并初始化WASI结构体
    struct w2c_wasi__unstable wasi;
    wasi.args_sizes_get = w2c_wasi__unstable_args_sizes_get;
    wasi.args_get = w2c_wasi__unstable_args_get;
    wasi.fd_write = w2c_wasi__unstable_fd_write;
    wasi.fd_read = w2c_wasi__unstable_fd_read;
    wasi.path_open = w2c_wasi__unstable_path_open;
    wasi.proc_exit = w2c_wasi__unstable_proc_exit;
    
    wasm2c_tee_instantiate(&instance, &wasi);
    
    w2c_tee_0x5Fstart(&instance);
    
    wasm2c_tee_free(&instance);
    wasm_rt_free();
    return 0;
}