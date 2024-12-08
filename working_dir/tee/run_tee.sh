wat2wasm tee.wast -o tee.wasm
~/workspace/course_project/wabt/bin/wasm2c tee.wasm -o tee.c
gcc tee.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -lm \
    -o tee
echo "Hello world" | ./tee output.txt