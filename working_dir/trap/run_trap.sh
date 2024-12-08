wat2wasm trap.wast -o trap.wasm
~/workspace/course_project/wabt/bin/wasm2c trap.wasm -o trap.c
gcc trap.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -lm \
    -o trap
./trap