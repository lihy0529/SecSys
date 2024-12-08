wat2wasm helloworld.wast -o hello.wasm
~/workspace/course_project/wabt/bin/wasm2c hello.wasm -o hello.c
gcc hello.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -lm \
    -o hello
./hello