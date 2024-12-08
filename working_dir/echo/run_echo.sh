wat2wasm echo.wast -o echo.wasm
~/workspace/course_project/wabt/bin/wasm2c echo.wasm -o echo.c
gcc echo.c main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -lm \
    -o echo
./echo hello_world