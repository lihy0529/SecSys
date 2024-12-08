import matplotlib.pyplot as plt
import numpy as np

# 数据准备
sizes = [32, 64, 128, 256, 512, 1024, 2048]

# 主要实现方式的性能数据
raw_c = [0.000327, 0.000530, 0.002343, 0.012651, 0.029102, 0.223657, 1.688340]
raw_c_valgrind = [0.002073, 0.004439, 0.036164, 0.269000, 2.133587, 16.826041, 135.658864]
raw_c_sanitizer = [0.000489, 0.001636, 0.008303, 0.024968, 0.191951, 1.523639, 36.765491]
wasm2c = [0.000657, 0.003178, 0.013571, 0.049218, 0.381216, 3.018055, 29.755940]
nodejs = [0.000661, 0.000831, 0.005909, 0.044815, 0.354009, 2.793808, 29.968730]
wavm = [0.031178, 0.026434, 0.032654, 0.075165, 0.382555, 2.878711, 29.632782]
wasmtime_no_guard = [0.000251, 0.001301, 0.008925, 0.054592, 0.426257, 3.295156, 32.102236]
wasmtime_guard = [0.000170, 0.000688, 0.007484, 0.040276, 0.308240, 2.440347, 29.674521]

# wasm2c不同优化级别的数据
wasm2c_O0 = [0.009529, 0.028273, 0.215147, 1.731330, 13.408999, 106.605372, 852.088526]
wasm2c_O1 = [0.000575, 0.002410, 0.013976, 0.055189, 0.427288, 3.386982, 31.217496]
wasm2c_O2 = [0.000675, 0.003162, 0.012943, 0.049317, 0.381854, 3.007312, 29.441884]
wasm2c_O3 = [0.000657, 0.003178, 0.013571, 0.049218, 0.381216, 3.018055, 29.755940]

# 创建图1：主要实现方式对比
plt.figure(figsize=(12, 6))
plt.plot(sizes, raw_c, 'o-', label='Raw C')
plt.plot(sizes, raw_c_valgrind, 's-', label='Raw C (valgrind)')
plt.plot(sizes, raw_c_sanitizer, '^-', label='Raw C (sanitizer)')
plt.plot(sizes, wasm2c, 'D-', label='wasm2c')
plt.plot(sizes, nodejs, 'v-', label='WebAssembly in Node.js')
plt.plot(sizes, wavm, '<-', label='WAVM')
plt.plot(sizes, wasmtime_guard, 'p-', label='Wasmtime (with guard)')

plt.xscale('log', base=2)
plt.yscale('log')
plt.grid(True)
plt.xlabel('Matrix Size N', fontsize=14)
plt.ylabel('Time (seconds)', fontsize=14)
plt.title('Evaluation on Matrix Multiplication C_N = A_N * B_N', fontsize=14)
plt.legend(bbox_to_anchor=(0, 1), loc='upper left', fontsize=14)

# 调整布局以确保图例完全可见
plt.tight_layout()
plt.savefig('fig/perf_comparison.pdf', bbox_inches='tight')

# 创建图2：wasm2c优化级别对比
plt.figure(figsize=(12, 6))
plt.plot(sizes, wasm2c_O0, 'o-', label='wasm2c -O0')
plt.plot(sizes, wasm2c_O1, 's-', label='wasm2c -O1')
plt.plot(sizes, wasm2c_O2, '^-', label='wasm2c -O2')
plt.plot(sizes, wasm2c_O3, 'D-', label='wasm2c -O3')

plt.xscale('log', base=2)
plt.yscale('log')
plt.grid(True)
plt.xlabel('Input Size')
plt.ylabel('Time (seconds)')
plt.title('wasm2c Performance with Different Optimization Levels')
plt.legend()
plt.savefig('fig/wasm2c_optimization_comparison.pdf', bbox_inches='tight')

plt.show()