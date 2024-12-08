wat2wasm matrix.wast -o matrix.wasm
~/workspace/course_project/wabt/bin/wasm2c matrix.wasm -o matrix.c
gcc matrix.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -lm \
    -o matrix
./matrix