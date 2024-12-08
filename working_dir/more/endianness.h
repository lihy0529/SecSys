/* Automatically generated by wasm2c */
#ifndef ENDIANNESS_H_GENERATED_
#define ENDIANNESS_H_GENERATED_

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

typedef struct w2c_endianness {
  wasm_rt_memory_t w2c_M0;
} w2c_endianness;

void wasm2c_endianness_instantiate(w2c_endianness*);
void wasm2c_endianness_free(w2c_endianness*);
wasm_rt_func_type_t wasm2c_endianness_get_func_type(uint32_t param_count, uint32_t result_count, ...);

/* export: 'i32_load16_s' */
u32 w2c_endianness_i32_load16_s(w2c_endianness*, u32);

/* export: 'i32_load16_u' */
u32 w2c_endianness_i32_load16_u(w2c_endianness*, u32);

/* export: 'i32_load' */
u32 w2c_endianness_i32_load(w2c_endianness*, u32);

/* export: 'i64_load16_s' */
u64 w2c_endianness_i64_load16_s(w2c_endianness*, u64);

/* export: 'i64_load16_u' */
u64 w2c_endianness_i64_load16_u(w2c_endianness*, u64);

/* export: 'i64_load32_s' */
u64 w2c_endianness_i64_load32_s(w2c_endianness*, u64);

/* export: 'i64_load32_u' */
u64 w2c_endianness_i64_load32_u(w2c_endianness*, u64);

/* export: 'i64_load' */
u64 w2c_endianness_i64_load(w2c_endianness*, u64);

/* export: 'f32_load' */
f32 w2c_endianness_f32_load(w2c_endianness*, f32);

/* export: 'f64_load' */
f64 w2c_endianness_f64_load(w2c_endianness*, f64);

/* export: 'i32_store16' */
u32 w2c_endianness_i32_store16(w2c_endianness*, u32);

/* export: 'i32_store' */
u32 w2c_endianness_i32_store(w2c_endianness*, u32);

/* export: 'i64_store16' */
u64 w2c_endianness_i64_store16(w2c_endianness*, u64);

/* export: 'i64_store32' */
u64 w2c_endianness_i64_store32(w2c_endianness*, u64);

/* export: 'i64_store' */
u64 w2c_endianness_i64_store(w2c_endianness*, u64);

/* export: 'f32_store' */
f32 w2c_endianness_f32_store(w2c_endianness*, f32);

/* export: 'f64_store' */
f64 w2c_endianness_f64_store(w2c_endianness*, f64);

#ifdef __cplusplus
}
#endif

#endif  /* ENDIANNESS_H_GENERATED_ */
