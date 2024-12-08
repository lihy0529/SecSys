/* Automatically generated by wasm2c */
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#if defined(__MINGW32__)
#include <malloc.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#include <malloc.h>
#define alloca _alloca
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#include <stdlib.h>
#else
#include <alloca.h>
#endif

#include "echo.h"
#define IS_SINGLE_UNSHARED_MEMORY 1

// Computes a pointer to an object of the given size in a little-endian memory.
//
// On a little-endian host, this is just &mem->data[addr] - the object's size is
// unused. On a big-endian host, it's &mem->data[mem->size - addr - n], where n
// is the object's size.
//
// Note that mem may be evaluated multiple times.
//
// Parameters:
// mem - The memory.
// addr - The address.
// n - The size of the object.
//
// Result:
// A pointer for an object of size n.
#if WABT_BIG_ENDIAN
#define MEM_ADDR(mem, addr, n) &(mem)->data[(mem)->size - (addr) - (n)]
#else
#define MEM_ADDR(mem, addr, n) &(mem)->data[addr]
#endif

#ifndef WASM_RT_USE_SEGUE
// Memory functions can use the segue optimization if allowed. The segue
// optimization uses x86 segments to point to a linear memory. We use this
// optimization when:
//
// (1) Segue is allowed using WASM_RT_ALLOW_SEGUE
// (2) on x86_64 without WABT_BIG_ENDIAN enabled
// (3) the Wasm module uses a single unshared imported or exported memory
// (4) the compiler supports: intrinsics for (rd|wr)gsbase, "address namespaces"
//     for accessing pointers, and supports memcpy on pointers with custom
//     "address namespaces". GCC does not support the memcpy requirement, so
//     this leaves only clang for now.
// (5) The OS provides a way to query if (rd|wr)gsbase is allowed by the kernel
// or the implementation has to use a syscall for this.
// (6) The OS doesn't replace the segment register on context switch which
//     eliminates windows for now
//
// While more OS can be supported in the future, we only support linux for now
#if WASM_RT_ALLOW_SEGUE && !WABT_BIG_ENDIAN &&                               \
    (defined(__x86_64__) || defined(_M_X64)) && IS_SINGLE_UNSHARED_MEMORY && \
    __clang__ && __has_builtin(__builtin_ia32_wrgsbase64) &&                 \
    !defined(_WIN32) && defined(__linux__)
#define WASM_RT_USE_SEGUE 1
#else
#define WASM_RT_USE_SEGUE 0
#endif
#endif

#if WASM_RT_USE_SEGUE
// POSIX uses FS for TLS, GS is free
static inline void* wasm_rt_segue_read_base() {
  if (wasm_rt_fsgsbase_inst_supported) {
    return (void*)__builtin_ia32_rdgsbase64();
  } else {
    return wasm_rt_syscall_get_segue_base();
  }
}
static inline void wasm_rt_segue_write_base(void* base) {
  if (wasm_rt_fsgsbase_inst_supported) {
    __builtin_ia32_wrgsbase64((uintptr_t)base);
  } else {
    wasm_rt_syscall_set_segue_base(base);
  }
}
#define MEM_ADDR_MEMOP(mem, addr, n) ((uint8_t __seg_gs*)(uintptr_t)addr)
#else
#define MEM_ADDR_MEMOP(mem, addr, n) MEM_ADDR(mem, addr, n)
#endif

#define TRAP(x) (wasm_rt_trap(WASM_RT_TRAP_##x), 0)

#if WASM_RT_STACK_DEPTH_COUNT
#define FUNC_PROLOGUE                                            \
  if (++wasm_rt_call_stack_depth > WASM_RT_MAX_CALL_STACK_DEPTH) \
    TRAP(EXHAUSTION);

#define FUNC_EPILOGUE --wasm_rt_call_stack_depth
#else
#define FUNC_PROLOGUE

#define FUNC_EPILOGUE
#endif

#define UNREACHABLE TRAP(UNREACHABLE)

static inline bool func_types_eq(const wasm_rt_func_type_t a,
                                 const wasm_rt_func_type_t b) {
  return (a == b) || LIKELY(a && b && !memcmp(a, b, 32));
}

#define CHECK_CALL_INDIRECT(table, ft, x)                \
  (LIKELY((x) < table.size && table.data[x].func &&      \
          func_types_eq(ft, table.data[x].func_type)) || \
   TRAP(CALL_INDIRECT))

#define DO_CALL_INDIRECT(table, t, x, ...) ((t)table.data[x].func)(__VA_ARGS__)

#define CALL_INDIRECT(table, t, ft, x, ...) \
  (CHECK_CALL_INDIRECT(table, ft, x),       \
   DO_CALL_INDIRECT(table, t, x, __VA_ARGS__))

#ifdef SUPPORT_MEMORY64
#define RANGE_CHECK(mem, offset, len)              \
  do {                                             \
    uint64_t res;                                  \
    if (__builtin_add_overflow(offset, len, &res)) \
      TRAP(OOB);                                   \
    if (UNLIKELY(res > mem->size))                 \
      TRAP(OOB);                                   \
  } while (0);
#else
#define RANGE_CHECK(mem, offset, len)               \
  if (UNLIKELY(offset + (uint64_t)len > mem->size)) \
    TRAP(OOB);
#endif

#if WASM_RT_USE_SEGUE && WASM_RT_SANITY_CHECKS
#include <stdio.h>
#define WASM_RT_CHECK_BASE(mem)                                               \
  if (((uintptr_t)((mem)->data)) != ((uintptr_t)wasm_rt_segue_read_base())) { \
    puts("Segment register mismatch\n");                                      \
    abort();                                                                  \
  }
#else
#define WASM_RT_CHECK_BASE(mem)
#endif

#if WASM_RT_MEMCHECK_GUARD_PAGES
#define MEMCHECK(mem, a, t) WASM_RT_CHECK_BASE(mem);
#else
#define MEMCHECK(mem, a, t) \
  WASM_RT_CHECK_BASE(mem);  \
  RANGE_CHECK(mem, a, sizeof(t))
#endif

#ifdef __GNUC__
#define FORCE_READ_INT(var) __asm__("" ::"r"(var));
// Clang on Mips requires "f" constraints on floats
// See https://github.com/llvm/llvm-project/issues/64241
#if defined(__clang__) && \
    (defined(mips) || defined(__mips__) || defined(__mips))
#define FORCE_READ_FLOAT(var) __asm__("" ::"f"(var));
#else
#define FORCE_READ_FLOAT(var) __asm__("" ::"r"(var));
#endif
#else
#define FORCE_READ_INT(var)
#define FORCE_READ_FLOAT(var)
#endif

static inline void load_data(void* dest, const void* src, size_t n) {
  if (!n) {
    return;
  }
  wasm_rt_memcpy(dest, src, n);
#if WABT_BIG_ENDIAN
  u8* dest_chars = dest;
  for (size_t i = 0; i < (n >> 1); i++) {
    u8 cursor = dest_chars[i];
    dest_chars[i] = dest_chars[n - i - 1];
    dest_chars[n - i - 1] = cursor;
  }
#endif
}

#define LOAD_DATA(m, o, i, s)            \
  do {                                   \
    RANGE_CHECK((&m), o, s);             \
    load_data(MEM_ADDR(&m, o, s), i, s); \
  } while (0)

#define DEFINE_LOAD(name, t1, t2, t3, force_read)                  \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {         \
    MEMCHECK(mem, addr, t1);                                       \
    t1 result;                                                     \
    wasm_rt_memcpy(&result, MEM_ADDR_MEMOP(mem, addr, sizeof(t1)), \
                   sizeof(t1));                                    \
    force_read(result);                                            \
    return (t3)(t2)result;                                         \
  }

#define DEFINE_STORE(name, t1, t2)                                     \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    MEMCHECK(mem, addr, t1);                                           \
    t1 wrapped = (t1)value;                                            \
    wasm_rt_memcpy(MEM_ADDR_MEMOP(mem, addr, sizeof(t1)), &wrapped,    \
                   sizeof(t1));                                        \
  }

DEFINE_LOAD(i32_load, u32, u32, u32, FORCE_READ_INT)
DEFINE_LOAD(i64_load, u64, u64, u64, FORCE_READ_INT)
DEFINE_LOAD(f32_load, f32, f32, f32, FORCE_READ_FLOAT)
DEFINE_LOAD(f64_load, f64, f64, f64, FORCE_READ_FLOAT)
DEFINE_LOAD(i32_load8_s, s8, s32, u32, FORCE_READ_INT)
DEFINE_LOAD(i64_load8_s, s8, s64, u64, FORCE_READ_INT)
DEFINE_LOAD(i32_load8_u, u8, u32, u32, FORCE_READ_INT)
DEFINE_LOAD(i64_load8_u, u8, u64, u64, FORCE_READ_INT)
DEFINE_LOAD(i32_load16_s, s16, s32, u32, FORCE_READ_INT)
DEFINE_LOAD(i64_load16_s, s16, s64, u64, FORCE_READ_INT)
DEFINE_LOAD(i32_load16_u, u16, u32, u32, FORCE_READ_INT)
DEFINE_LOAD(i64_load16_u, u16, u64, u64, FORCE_READ_INT)
DEFINE_LOAD(i64_load32_s, s32, s64, u64, FORCE_READ_INT)
DEFINE_LOAD(i64_load32_u, u32, u64, u64, FORCE_READ_INT)
DEFINE_STORE(i32_store, u32, u32)
DEFINE_STORE(i64_store, u64, u64)
DEFINE_STORE(f32_store, f32, f32)
DEFINE_STORE(f64_store, f64, f64)
DEFINE_STORE(i32_store8, u8, u32)
DEFINE_STORE(i32_store16, u16, u32)
DEFINE_STORE(i64_store8, u8, u64)
DEFINE_STORE(i64_store16, u16, u64)
DEFINE_STORE(i64_store32, u32, u64)

#if defined(_MSC_VER)

// Adapted from
// https://github.com/nemequ/portable-snippets/blob/master/builtin/builtin.h

static inline int I64_CLZ(unsigned long long v) {
  unsigned long r = 0;
#if defined(_M_AMD64) || defined(_M_ARM)
  if (_BitScanReverse64(&r, v)) {
    return 63 - r;
  }
#else
  if (_BitScanReverse(&r, (unsigned long)(v >> 32))) {
    return 31 - r;
  } else if (_BitScanReverse(&r, (unsigned long)v)) {
    return 63 - r;
  }
#endif
  return 64;
}

static inline int I32_CLZ(unsigned long v) {
  unsigned long r = 0;
  if (_BitScanReverse(&r, v)) {
    return 31 - r;
  }
  return 32;
}

static inline int I64_CTZ(unsigned long long v) {
  if (!v) {
    return 64;
  }
  unsigned long r = 0;
#if defined(_M_AMD64) || defined(_M_ARM)
  _BitScanForward64(&r, v);
  return (int)r;
#else
  if (_BitScanForward(&r, (unsigned int)(v))) {
    return (int)(r);
  }

  _BitScanForward(&r, (unsigned int)(v >> 32));
  return (int)(r + 32);
#endif
}

static inline int I32_CTZ(unsigned long v) {
  if (!v) {
    return 32;
  }
  unsigned long r = 0;
  _BitScanForward(&r, v);
  return (int)r;
}

#define POPCOUNT_DEFINE_PORTABLE(f_n, T)                            \
  static inline u32 f_n(T x) {                                      \
    x = x - ((x >> 1) & (T) ~(T)0 / 3);                             \
    x = (x & (T) ~(T)0 / 15 * 3) + ((x >> 2) & (T) ~(T)0 / 15 * 3); \
    x = (x + (x >> 4)) & (T) ~(T)0 / 255 * 15;                      \
    return (T)(x * ((T) ~(T)0 / 255)) >> (sizeof(T) - 1) * 8;       \
  }

POPCOUNT_DEFINE_PORTABLE(I32_POPCNT, u32)
POPCOUNT_DEFINE_PORTABLE(I64_POPCNT, u64)

#undef POPCOUNT_DEFINE_PORTABLE

#else

#define I32_CLZ(x) ((x) ? __builtin_clz(x) : 32)
#define I64_CLZ(x) ((x) ? __builtin_clzll(x) : 64)
#define I32_CTZ(x) ((x) ? __builtin_ctz(x) : 32)
#define I64_CTZ(x) ((x) ? __builtin_ctzll(x) : 64)
#define I32_POPCNT(x) (__builtin_popcount(x))
#define I64_POPCNT(x) (__builtin_popcountll(x))

#endif

#define DIV_S(ut, min, x, y)                                      \
  ((UNLIKELY((y) == 0))                                           \
       ? TRAP(DIV_BY_ZERO)                                        \
       : (UNLIKELY((x) == min && (y) == -1)) ? TRAP(INT_OVERFLOW) \
                                             : (ut)((x) / (y)))

#define REM_S(ut, min, x, y) \
  ((UNLIKELY((y) == 0))      \
       ? TRAP(DIV_BY_ZERO)   \
       : (UNLIKELY((x) == min && (y) == -1)) ? 0 : (ut)((x) % (y)))

#define I32_DIV_S(x, y) DIV_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_DIV_S(x, y) DIV_S(u64, INT64_MIN, (s64)x, (s64)y)
#define I32_REM_S(x, y) REM_S(u32, INT32_MIN, (s32)x, (s32)y)
#define I64_REM_S(x, y) REM_S(u64, INT64_MIN, (s64)x, (s64)y)

#define DIVREM_U(op, x, y) \
  ((UNLIKELY((y) == 0)) ? TRAP(DIV_BY_ZERO) : ((x)op(y)))

#define DIV_U(x, y) DIVREM_U(/, x, y)
#define REM_U(x, y) DIVREM_U(%, x, y)

#define ROTL(x, y, mask) \
  (((x) << ((y) & (mask))) | ((x) >> (((mask) - (y) + 1) & (mask))))
#define ROTR(x, y, mask) \
  (((x) >> ((y) & (mask))) | ((x) << (((mask) - (y) + 1) & (mask))))

#define I32_ROTL(x, y) ROTL(x, y, 31)
#define I64_ROTL(x, y) ROTL(x, y, 63)
#define I32_ROTR(x, y) ROTR(x, y, 31)
#define I64_ROTR(x, y) ROTR(x, y, 63)

#define FMIN(x, y)                                                     \
  ((UNLIKELY((x) != (x)))                                              \
       ? NAN                                                           \
       : (UNLIKELY((y) != (y)))                                        \
             ? NAN                                                     \
             : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? x : y) \
                                                : (x < y) ? x : y)

#define FMAX(x, y)                                                     \
  ((UNLIKELY((x) != (x)))                                              \
       ? NAN                                                           \
       : (UNLIKELY((y) != (y)))                                        \
             ? NAN                                                     \
             : (UNLIKELY((x) == 0 && (y) == 0)) ? (signbit(x) ? y : x) \
                                                : (x > y) ? x : y)

#define TRUNC_S(ut, st, ft, min, minop, max, x)                           \
  ((UNLIKELY((x) != (x)))                                                 \
       ? TRAP(INVALID_CONVERSION)                                         \
       : (UNLIKELY(!((x)minop(min) && (x) < (max)))) ? TRAP(INT_OVERFLOW) \
                                                     : (ut)(st)(x))

#define I32_TRUNC_S_F32(x) \
  TRUNC_S(u32, s32, f32, (f32)INT32_MIN, >=, 2147483648.f, x)
#define I64_TRUNC_S_F32(x) \
  TRUNC_S(u64, s64, f32, (f32)INT64_MIN, >=, (f32)INT64_MAX, x)
#define I32_TRUNC_S_F64(x) \
  TRUNC_S(u32, s32, f64, -2147483649., >, 2147483648., x)
#define I64_TRUNC_S_F64(x) \
  TRUNC_S(u64, s64, f64, (f64)INT64_MIN, >=, (f64)INT64_MAX, x)

#define TRUNC_U(ut, ft, max, x)                                          \
  ((UNLIKELY((x) != (x)))                                                \
       ? TRAP(INVALID_CONVERSION)                                        \
       : (UNLIKELY(!((x) > (ft)-1 && (x) < (max)))) ? TRAP(INT_OVERFLOW) \
                                                    : (ut)(x))

#define I32_TRUNC_U_F32(x) TRUNC_U(u32, f32, 4294967296.f, x)
#define I64_TRUNC_U_F32(x) TRUNC_U(u64, f32, (f32)UINT64_MAX, x)
#define I32_TRUNC_U_F64(x) TRUNC_U(u32, f64, 4294967296., x)
#define I64_TRUNC_U_F64(x) TRUNC_U(u64, f64, (f64)UINT64_MAX, x)

#define TRUNC_SAT_S(ut, st, ft, min, smin, minop, max, smax, x) \
  ((UNLIKELY((x) != (x)))                                       \
       ? 0                                                      \
       : (UNLIKELY(!((x)minop(min))))                           \
             ? smin                                             \
             : (UNLIKELY(!((x) < (max)))) ? smax : (ut)(st)(x))

#define I32_TRUNC_SAT_S_F32(x)                                            \
  TRUNC_SAT_S(u32, s32, f32, (f32)INT32_MIN, INT32_MIN, >=, 2147483648.f, \
              INT32_MAX, x)
#define I64_TRUNC_SAT_S_F32(x)                                              \
  TRUNC_SAT_S(u64, s64, f32, (f32)INT64_MIN, INT64_MIN, >=, (f32)INT64_MAX, \
              INT64_MAX, x)
#define I32_TRUNC_SAT_S_F64(x)                                        \
  TRUNC_SAT_S(u32, s32, f64, -2147483649., INT32_MIN, >, 2147483648., \
              INT32_MAX, x)
#define I64_TRUNC_SAT_S_F64(x)                                              \
  TRUNC_SAT_S(u64, s64, f64, (f64)INT64_MIN, INT64_MIN, >=, (f64)INT64_MAX, \
              INT64_MAX, x)

#define TRUNC_SAT_U(ut, ft, max, smax, x)               \
  ((UNLIKELY((x) != (x))) ? 0                           \
                          : (UNLIKELY(!((x) > (ft)-1))) \
                                ? 0                     \
                                : (UNLIKELY(!((x) < (max)))) ? smax : (ut)(x))

#define I32_TRUNC_SAT_U_F32(x) \
  TRUNC_SAT_U(u32, f32, 4294967296.f, UINT32_MAX, x)
#define I64_TRUNC_SAT_U_F32(x) \
  TRUNC_SAT_U(u64, f32, (f32)UINT64_MAX, UINT64_MAX, x)
#define I32_TRUNC_SAT_U_F64(x) TRUNC_SAT_U(u32, f64, 4294967296., UINT32_MAX, x)
#define I64_TRUNC_SAT_U_F64(x) \
  TRUNC_SAT_U(u64, f64, (f64)UINT64_MAX, UINT64_MAX, x)

#define DEFINE_REINTERPRET(name, t1, t2)         \
  static inline t2 name(t1 x) {                  \
    t2 result;                                   \
    wasm_rt_memcpy(&result, &x, sizeof(result)); \
    return result;                               \
  }

DEFINE_REINTERPRET(f32_reinterpret_i32, u32, f32)
DEFINE_REINTERPRET(i32_reinterpret_f32, f32, u32)
DEFINE_REINTERPRET(f64_reinterpret_i64, u64, f64)
DEFINE_REINTERPRET(i64_reinterpret_f64, f64, u64)

static float quiet_nanf(float x) {
  uint32_t tmp;
  wasm_rt_memcpy(&tmp, &x, 4);
  tmp |= 0x7fc00000lu;
  wasm_rt_memcpy(&x, &tmp, 4);
  return x;
}

static double quiet_nan(double x) {
  uint64_t tmp;
  wasm_rt_memcpy(&tmp, &x, 8);
  tmp |= 0x7ff8000000000000llu;
  wasm_rt_memcpy(&x, &tmp, 8);
  return x;
}

static double wasm_quiet(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return x;
}

static float wasm_quietf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return x;
}

static double wasm_floor(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return floor(x);
}

static float wasm_floorf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return floorf(x);
}

static double wasm_ceil(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return ceil(x);
}

static float wasm_ceilf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return ceilf(x);
}

static double wasm_trunc(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return trunc(x);
}

static float wasm_truncf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return truncf(x);
}

static float wasm_nearbyintf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return nearbyintf(x);
}

static double wasm_nearbyint(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return nearbyint(x);
}

static float wasm_fabsf(float x) {
  if (UNLIKELY(isnan(x))) {
    uint32_t tmp;
    wasm_rt_memcpy(&tmp, &x, 4);
    tmp = tmp & ~(1UL << 31);
    wasm_rt_memcpy(&x, &tmp, 4);
    return x;
  }
  return fabsf(x);
}

static double wasm_fabs(double x) {
  if (UNLIKELY(isnan(x))) {
    uint64_t tmp;
    wasm_rt_memcpy(&tmp, &x, 8);
    tmp = tmp & ~(1ULL << 63);
    wasm_rt_memcpy(&x, &tmp, 8);
    return x;
  }
  return fabs(x);
}

static double wasm_sqrt(double x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nan(x);
  }
  return sqrt(x);
}

static float wasm_sqrtf(float x) {
  if (UNLIKELY(isnan(x))) {
    return quiet_nanf(x);
  }
  return sqrtf(x);
}

static inline void memory_fill(wasm_rt_memory_t* mem, u32 d, u32 val, u32 n) {
  RANGE_CHECK(mem, d, n);
  memset(MEM_ADDR(mem, d, n), val, n);
}

static inline void memory_copy(wasm_rt_memory_t* dest,
                               const wasm_rt_memory_t* src,
                               u32 dest_addr,
                               u32 src_addr,
                               u32 n) {
  RANGE_CHECK(dest, dest_addr, n);
  RANGE_CHECK(src, src_addr, n);
  memmove(MEM_ADDR(dest, dest_addr, n), MEM_ADDR(src, src_addr, n), n);
}

static inline void memory_init(wasm_rt_memory_t* dest,
                               const u8* src,
                               u32 src_size,
                               u32 dest_addr,
                               u32 src_addr,
                               u32 n) {
  if (UNLIKELY(src_addr + (uint64_t)n > src_size))
    TRAP(OOB);
  LOAD_DATA((*dest), dest_addr, src + src_addr, n);
}

typedef struct {
  enum { RefFunc, RefNull, GlobalGet } expr_type;
  wasm_rt_func_type_t type;
  wasm_rt_function_ptr_t func;
  wasm_rt_tailcallee_t func_tailcallee;
  size_t module_offset;
} wasm_elem_segment_expr_t;

static inline void funcref_table_init(wasm_rt_funcref_table_t* dest,
                                      const wasm_elem_segment_expr_t* src,
                                      u32 src_size,
                                      u32 dest_addr,
                                      u32 src_addr,
                                      u32 n,
                                      void* module_instance) {
  if (UNLIKELY(src_addr + (uint64_t)n > src_size))
    TRAP(OOB);
  if (UNLIKELY(dest_addr + (uint64_t)n > dest->size))
    TRAP(OOB);
  for (u32 i = 0; i < n; i++) {
    const wasm_elem_segment_expr_t* const src_expr = &src[src_addr + i];
    wasm_rt_funcref_t* const dest_val = &(dest->data[dest_addr + i]);
    switch (src_expr->expr_type) {
      case RefFunc:
        *dest_val = (wasm_rt_funcref_t){
            src_expr->type, src_expr->func, src_expr->func_tailcallee,
            (char*)module_instance + src_expr->module_offset};
        break;
      case RefNull:
        *dest_val = wasm_rt_funcref_null_value;
        break;
      case GlobalGet:
        *dest_val = **(wasm_rt_funcref_t**)((char*)module_instance +
                                            src_expr->module_offset);
        break;
    }
  }
}

// Currently wasm2c only supports initializing externref tables with ref.null.
static inline void externref_table_init(wasm_rt_externref_table_t* dest,
                                        u32 src_size,
                                        u32 dest_addr,
                                        u32 src_addr,
                                        u32 n) {
  if (UNLIKELY(src_addr + (uint64_t)n > src_size))
    TRAP(OOB);
  if (UNLIKELY(dest_addr + (uint64_t)n > dest->size))
    TRAP(OOB);
  for (u32 i = 0; i < n; i++) {
    dest->data[dest_addr + i] = wasm_rt_externref_null_value;
  }
}

#define DEFINE_TABLE_COPY(type)                                              \
  static inline void type##_table_copy(wasm_rt_##type##_table_t* dest,       \
                                       const wasm_rt_##type##_table_t* src,  \
                                       u32 dest_addr, u32 src_addr, u32 n) { \
    if (UNLIKELY(dest_addr + (uint64_t)n > dest->size))                      \
      TRAP(OOB);                                                             \
    if (UNLIKELY(src_addr + (uint64_t)n > src->size))                        \
      TRAP(OOB);                                                             \
                                                                             \
    memmove(dest->data + dest_addr, src->data + src_addr,                    \
            n * sizeof(wasm_rt_##type##_t));                                 \
  }

DEFINE_TABLE_COPY(funcref)
DEFINE_TABLE_COPY(externref)

#define DEFINE_TABLE_GET(type)                        \
  static inline wasm_rt_##type##_t type##_table_get(  \
      const wasm_rt_##type##_table_t* table, u32 i) { \
    if (UNLIKELY(i >= table->size))                   \
      TRAP(OOB);                                      \
    return table->data[i];                            \
  }

DEFINE_TABLE_GET(funcref)
DEFINE_TABLE_GET(externref)

#define DEFINE_TABLE_SET(type)                                               \
  static inline void type##_table_set(const wasm_rt_##type##_table_t* table, \
                                      u32 i, const wasm_rt_##type##_t val) { \
    if (UNLIKELY(i >= table->size))                                          \
      TRAP(OOB);                                                             \
    table->data[i] = val;                                                    \
  }

DEFINE_TABLE_SET(funcref)
DEFINE_TABLE_SET(externref)

#define DEFINE_TABLE_FILL(type)                                               \
  static inline void type##_table_fill(const wasm_rt_##type##_table_t* table, \
                                       u32 d, const wasm_rt_##type##_t val,   \
                                       u32 n) {                               \
    if (UNLIKELY((uint64_t)d + n > table->size))                              \
      TRAP(OOB);                                                              \
    for (uint32_t i = d; i < d + n; i++) {                                    \
      table->data[i] = val;                                                   \
    }                                                                         \
  }

DEFINE_TABLE_FILL(funcref)
DEFINE_TABLE_FILL(externref)

#if defined(__GNUC__) || defined(__clang__)
#define FUNC_TYPE_DECL_EXTERN_T(x) extern const char* const x
#define FUNC_TYPE_EXTERN_T(x) const char* const x
#define FUNC_TYPE_T(x) static const char* const x
#else
#define FUNC_TYPE_DECL_EXTERN_T(x) extern const char x[]
#define FUNC_TYPE_EXTERN_T(x) const char x[]
#define FUNC_TYPE_T(x) static const char x[]
#endif

#if (__STDC_VERSION__ < 201112L) && !defined(static_assert)
#define static_assert(X) \
  extern int(*assertion(void))[!!sizeof(struct { int x : (X) ? 2 : -1; })];
#endif

#ifdef _MSC_VER
#define WEAK_FUNC_DECL(func, fallback)                             \
  __pragma(comment(linker, "/alternatename:" #func "=" #fallback)) \
                                                                   \
      void                                                         \
      fallback(void** instance_ptr, void* tail_call_stack,         \
               wasm_rt_tailcallee_t* next)
#else
#define WEAK_FUNC_DECL(func, fallback)                                        \
  __attribute__((weak)) void func(void** instance_ptr, void* tail_call_stack, \
                                  wasm_rt_tailcallee_t* next)
#endif

static u32 w2c_echo_f4(w2c_echo*, u32);
static u32 w2c_echo_f5(w2c_echo*, u32);
static void w2c_echo_0x5Fstart_0(w2c_echo*);

FUNC_TYPE_T(w2c_echo_t0) = "\x92\xfb\x6a\xdf\x49\x07\x0a\x83\xbe\x08\x02\x68\xcd\xf6\x95\x27\x4a\xc2\xf3\xe5\xe4\x7d\x29\x49\xe8\xed\x42\x92\x6a\x9d\xda\xf0";
FUNC_TYPE_T(w2c_echo_t1) = "\xf6\x98\x1b\xc6\x10\xda\xb7\xb2\x63\x37\xcd\xdc\x72\xca\xe9\x50\x00\x13\xba\x10\x6c\xde\x87\x27\x10\xf8\x86\x2f\xe3\xdb\x94\xe4";
FUNC_TYPE_T(w2c_echo_t2) = "\x89\x3a\x3d\x2c\x8f\x4d\x7f\x6d\x6c\x9d\x62\x67\x29\xaf\x3d\x44\x39\x8e\xc3\xf3\xe8\x51\xc1\x99\xb9\xdd\x9f\xd5\x3d\x1f\xd3\xe4";
FUNC_TYPE_T(w2c_echo_t3) = "\x07\x80\x96\x7a\x42\xf7\x3e\xe6\x70\x5c\x2f\xac\x83\xf5\x67\xd2\xa2\xa0\x69\x41\x5f\xf8\xe7\x96\x7f\x23\xab\x00\x03\x5f\x4a\x3c";
FUNC_TYPE_T(w2c_echo_t4) = "\x36\xa9\xe7\xf1\xc9\x5b\x82\xff\xb9\x97\x43\xe0\xc5\xc4\xce\x95\xd8\x3c\x9a\x43\x0a\xac\x59\xf8\x4e\xf3\xcb\xfa\xb6\x14\x50\x68";

static void init_globals(w2c_echo* instance) {
  instance->w2c_g0 = 0u;
  instance->w2c_g1 = 1u;
  instance->w2c_g2 = 4u;
  instance->w2c_g3 = 8u;
  instance->w2c_g4 = 12u;
  instance->w2c_g5 = 128u;
}

static const u8 data_segment_data_w2c_echo_d0[] = {
  0x20, 
};

static const u8 data_segment_data_w2c_echo_d1[] = {
  0x0a, 
};

static void init_memories(w2c_echo* instance) {
  wasm_rt_allocate_memory(&instance->w2c_memory, 1, 65536, 0);
  LOAD_DATA(instance->w2c_memory, 0u, data_segment_data_w2c_echo_d0, 1);
  LOAD_DATA(instance->w2c_memory, 1u, data_segment_data_w2c_echo_d1, 1);
}

static void init_data_instances(w2c_echo *instance) {
}

/* export: 'memory' */
wasm_rt_memory_t* w2c_echo_memory(w2c_echo* instance) {
  return &instance->w2c_memory;
}

/* export: '_start' */
void w2c_echo_0x5Fstart(w2c_echo* instance) {
#if WASM_RT_USE_SEGUE
  void* segue_saved_base = wasm_rt_segue_read_base();
  wasm_rt_segue_write_base(instance->w2c_memory.data);
#endif
  w2c_echo_0x5Fstart_0(instance);
#if WASM_RT_USE_SEGUE
  wasm_rt_segue_write_base(segue_saved_base);
#endif
}

static void init_instance_import(w2c_echo* instance, struct w2c_wasi__unstable* w2c_wasi__unstable_instance) {
  instance->w2c_wasi__unstable_instance = w2c_wasi__unstable_instance;
}

void wasm2c_echo_instantiate(w2c_echo* instance, struct w2c_wasi__unstable* w2c_wasi__unstable_instance) {
  assert(wasm_rt_is_initialized());
  init_instance_import(instance, w2c_wasi__unstable_instance);
  init_globals(instance);
  init_memories(instance);
#if WASM_RT_USE_SEGUE
  void* segue_saved_base = wasm_rt_segue_read_base();
  wasm_rt_segue_write_base(instance->w2c_memory.data);
#endif
  init_data_instances(instance);
#if WASM_RT_USE_SEGUE
  wasm_rt_segue_write_base(segue_saved_base);
#endif
}

void wasm2c_echo_free(w2c_echo* instance) {
  wasm_rt_free_memory(&instance->w2c_memory);
}

wasm_rt_func_type_t wasm2c_echo_get_func_type(uint32_t param_count, uint32_t result_count, ...) {
  va_list args;
  
  if (param_count == 2 && result_count == 1) {
    va_start(args, result_count);
    if (true && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32) {
      va_end(args);
      return w2c_echo_t0;
    }
    va_end(args);
  }
  
  if (param_count == 4 && result_count == 1) {
    va_start(args, result_count);
    if (true && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32) {
      va_end(args);
      return w2c_echo_t1;
    }
    va_end(args);
  }
  
  if (param_count == 1 && result_count == 0) {
    va_start(args, result_count);
    if (true && va_arg(args, wasm_rt_type_t) == WASM_RT_I32) {
      va_end(args);
      return w2c_echo_t2;
    }
    va_end(args);
  }
  
  if (param_count == 1 && result_count == 1) {
    va_start(args, result_count);
    if (true && va_arg(args, wasm_rt_type_t) == WASM_RT_I32 && va_arg(args, wasm_rt_type_t) == WASM_RT_I32) {
      va_end(args);
      return w2c_echo_t3;
    }
    va_end(args);
  }
  
  if (param_count == 0 && result_count == 0) {
    va_start(args, result_count);
    if (true) {
      va_end(args);
      return w2c_echo_t4;
    }
    va_end(args);
  }
  
  return NULL;
}

u32 w2c_echo_f4(w2c_echo* instance, u32 var_p0) {
  u32 var_l1 = 0;
  FUNC_PROLOGUE;
  u32 var_i0, var_i1;
  var_i0 = var_p0;
  var_i1 = 15u;
  var_i0 += var_i1;
  var_i1 = 4294967280u;
  var_i0 &= var_i1;
  var_p0 = var_i0;
  var_i0 = instance->w2c_g5;
  var_l1 = var_i0;
  var_i0 = var_l1;
  var_i1 = var_p0;
  var_i0 += var_i1;
  instance->w2c_g5 = var_i0;
  var_i0 = var_l1;
  FUNC_EPILOGUE;
  return var_i0;
}

u32 w2c_echo_f5(w2c_echo* instance, u32 var_p0) {
  u32 var_l1 = 0;
  FUNC_PROLOGUE;
  u32 var_i0, var_i1;
  var_i0 = var_p0;
  var_l1 = var_i0;
  var_L0: 
    var_i0 = 0u;
    var_i1 = var_l1;
    printf("[%d] ld %p \n", __LINE__, (void*)((u64)(var_i1) + 0u));
    var_i1 = i32_load8_u(&instance->w2c_memory, (u64)(var_i1));
    var_i0 = var_i0 == var_i1;
    if (var_i0) {goto var_B1;}
    var_i0 = var_l1;
    var_i1 = 1u;
    var_i0 += var_i1;
    var_l1 = var_i0;
    goto var_L0;
    var_B1:;
  var_i0 = var_l1;
  var_i1 = var_p0;
  var_i0 -= var_i1;
  goto var_Bfunc;
  var_Bfunc:;
  FUNC_EPILOGUE;
  return var_i0;
}

void w2c_echo_0x5Fstart_0(w2c_echo* instance) {
  u32 var_l0 = 0, var_l1 = 0, var_l2 = 0, var_l3 = 0, var_l4 = 0, var_l5 = 0, var_l6 = 0, var_l7 = 0, 
      var_l8 = 0, var_l9 = 0, var_l10 = 0;
  FUNC_PROLOGUE;
  u32 var_i0, var_i1, var_i2, var_i3, var_i4, var_i5;
  var_i0 = instance->w2c_g2;
  var_i1 = instance->w2c_g3;
  var_i0 = w2c_wasi__unstable_args_sizes_get(instance->w2c_wasi__unstable_instance, var_i0, var_i1);
  var_l0 = var_i0;
  var_i0 = var_l0;
  var_i1 = 0u;
  var_i0 = var_i0 != var_i1;
  if (var_i0) {goto var_B0;}
  var_i0 = instance->w2c_g2;
  printf("[%d] ld %p \n", __LINE__, (void*)((u64)(var_i0) + 0u));
  var_i0 = i32_load(&instance->w2c_memory, (u64)(var_i0));
  var_l1 = var_i0;
  var_i0 = instance->w2c_g3;
  printf("[%d] ld %p \n", __LINE__, (void*)((u64)(var_i0) + 0u));
  var_i0 = i32_load(&instance->w2c_memory, (u64)(var_i0));
  var_l2 = var_i0;
  var_i0 = var_l1;
  var_i1 = 4u;
  var_i0 *= var_i1;
  var_i0 = w2c_echo_f4(instance, var_i0);
  var_l3 = var_i0;
  var_i0 = var_l2;
  var_i0 = w2c_echo_f4(instance, var_i0);
  var_l4 = var_i0;
  var_i0 = var_l3;
  var_i1 = var_l4;
  var_i0 = w2c_wasi__unstable_args_get(instance->w2c_wasi__unstable_instance, var_i0, var_i1);
  var_l0 = var_i0;
  var_i0 = var_l0;
  var_i1 = 0u;
  var_i0 = var_i0 != var_i1;
  if (var_i0) {goto var_B0;}
  var_i0 = var_l1;
  var_i1 = 1u;
  var_i0 -= var_i1;
  var_i1 = 2u;
  var_i0 *= var_i1;
  var_l6 = var_i0;
  var_i0 = var_l6;
  var_i1 = 8u;
  var_i0 *= var_i1;
  var_i0 = w2c_echo_f4(instance, var_i0);
  var_l5 = var_i0;
  var_i0 = var_l5;
  var_l8 = var_i0;
  var_i0 = 1u;
  var_l7 = var_i0;
  var_L2: 
    var_i0 = var_l7;
    var_i1 = var_l1;
    var_i0 = var_i0 == var_i1;
    if (var_i0) {goto var_B1;}
    var_i0 = var_l3;
    var_i1 = var_l7;
    var_i2 = 4u;
    var_i1 *= var_i2;
    var_i0 += var_i1;
    printf("[%d] ld %p \n", __LINE__, (void*)((u64)(var_i0) + 0u));
    var_i0 = i32_load(&instance->w2c_memory, (u64)(var_i0));
    var_l9 = var_i0;
    var_i0 = var_l9;
    var_i0 = w2c_echo_f5(instance, var_i0);
    var_l10 = var_i0;
    var_i0 = var_l8;
    var_i1 = var_l9;
    printf("[%d] st %p\n", __LINE__, (void*)((u64)(var_i0) + 0u));
    i32_store(&instance->w2c_memory, (u64)(var_i0), var_i1);
    var_i0 = var_l8;
    var_i1 = var_l10;
    printf("[%d] st %p\n", __LINE__, (void*)((u64)(var_i0) + 4u));
    i32_store(&instance->w2c_memory, (u64)(var_i0) + 4, var_i1);
    var_i0 = var_l8;
    var_i1 = instance->w2c_g1;
    var_i2 = instance->w2c_g0;
    var_i3 = var_l1;
    var_i4 = var_l7;
    var_i5 = 1u;
    var_i4 += var_i5;
    var_i3 = var_i3 == var_i4;
    var_i1 = var_i3 ? var_i1 : var_i2;
    printf("[%d] st %p\n", __LINE__, (void*)((u64)(var_i0) + 8u));
    i32_store(&instance->w2c_memory, (u64)(var_i0) + 8, var_i1);
    var_i0 = var_l8;
    var_i1 = 1u;
    printf("[%d] st %p\n", __LINE__, (void*)((u64)(var_i0) + 12u));
    i32_store(&instance->w2c_memory, (u64)(var_i0) + 12, var_i1);
    var_i0 = var_l7;
    var_i1 = 1u;
    var_i0 += var_i1;
    var_l7 = var_i0;
    var_i0 = var_l8;
    var_i1 = 16u;
    var_i0 += var_i1;
    var_l8 = var_i0;
    goto var_L2;
  var_B1:;
  var_i0 = 1u;
  var_i1 = var_l5;
  var_i2 = var_l6;
  var_i3 = instance->w2c_g4;
  var_i0 = w2c_wasi__unstable_fd_write(instance->w2c_wasi__unstable_instance, var_i0, var_i1, var_i2, var_i3);
  var_l0 = var_i0;
  var_B0:;
  var_i0 = var_l0;
  w2c_wasi__unstable_proc_exit(instance->w2c_wasi__unstable_instance, var_i0);
  FUNC_EPILOGUE;
}
