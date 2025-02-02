/* Automatically generated by wasm2c */
#ifndef HELLO_H_GENERATED_
#define HELLO_H_GENERATED_

#include "wasm-rt.h"

#include <stdint.h>

#ifndef WASM_RT_CORE_TYPES_DEFINED
#define WASM_RT_CORE_TYPES_DEFINED
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct w2c_wasi__unstable;

typedef struct w2c_hello {
  struct w2c_wasi__unstable* w2c_wasi__unstable_instance;
  wasm_rt_memory_t w2c_memory;
} w2c_hello;

void wasm2c_hello_instantiate(w2c_hello*, struct w2c_wasi__unstable*);
void wasm2c_hello_free(w2c_hello*);
wasm_rt_func_type_t wasm2c_hello_get_func_type(uint32_t param_count, uint32_t result_count, ...);

/* import: 'wasi_unstable' 'fd_write' */
u32 w2c_wasi__unstable_fd_write(struct w2c_wasi__unstable*, u32, u32, u32, u32);

/* import: 'wasi_unstable' 'proc_exit' */
void w2c_wasi__unstable_proc_exit(struct w2c_wasi__unstable*, u32);

/* export: 'memory' */
wasm_rt_memory_t* w2c_hello_memory(w2c_hello* instance);

/* export: '_start' */
void w2c_hello_0x5Fstart(w2c_hello*);

#ifdef __cplusplus
}
#endif

#endif  /* HELLO_H_GENERATED_ */
