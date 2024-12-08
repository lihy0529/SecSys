
emcc raw_auto_debug.c -o auto_debug.wasm -s STANDALONE_WASM -s EXPORTED_FUNCTIONS='["_calculate"]' --no-entry
~/workspace/course_project/wabt/bin/wasm2c auto_debug.wasm -o auto_debug.c

# Compile the generated C file with main.c
gcc auto_debug.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -Wno-implicit-function-declaration \
    -Wno-incompatible-pointer-types \
    -Wno-pointer-to-int-cast \
    -lm \
    -o auto_debug

mkdir -p tmp
python3 auto_debug.py > tmp/llm_output.txt
