#!/bin/bash

# 编译 WAT 到 WASM
wat2wasm vulnerable.wast -o vulnerable.wasm

# 将 WASM 转换为 C
~/workspace/course_project/wabt/bin/wasm2c vulnerable.wasm -o vulnerable.c

# 编译最终的可执行文件
gcc vulnerable.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-pointer-to-int-cast \
    -Wno-incompatible-pointer-types \
    -Wno-format \
    -include stdio.h \
    -lm \
    -o vulnerable

# 运行程序
./vulnerable ""

./vulnerable "A"

./vulnerable "ABC"

./vulnerable "ABCDE"

./vulnerable "ABCDEFG"

./vulnerable "ABCDEFGHI"

./vulnerable "ABCDEFGHIJK"
