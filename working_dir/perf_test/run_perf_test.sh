

emcc raw_perf_test.c -o perf_test.wasm -s STANDALONE_WASM -s EXPORTED_FUNCTIONS='["_calculate"]' --no-entry
wasm2c perf_test.wasm -o perf_test.c

# Compile the generated C file with main.c
gcc -O3 perf_test.c wasm2c_main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -Wno-int-to-pointer-cast \
    -Wno-format \
    -Wno-implicit-function-declaration \
    -Wno-incompatible-pointer-types \
    -Wno-pointer-to-int-cast \
    -fno-optimize-sibling-calls -frounding-math -fsignaling-nans \
    -lm \
    -o wasm2c_perf_test

mkdir -p tmp
echo "perf test" > tmp/perf_test.txt


# raw C perf test
gcc -O3 raw_main.c -o raw_perf_test
echo "----raw C perf test----" >> tmp/perf_test.txt
./raw_perf_test >> tmp/perf_test.txt

echo "----raw C with valgrind----" >> tmp/perf_test.txt
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./raw_perf_test >> tmp/perf_test.txt

echo "----raw C with address sanitizer----" >> tmp/perf_test.txt
gcc -fsanitize=address -O3 raw_main.c -o raw_perf_test
./raw_perf_test >> tmp/perf_test.txt


# wasm2c perf test
echo "----wasm2c perf test----" >> tmp/perf_test.txt
./wasm2c_perf_test >> tmp/perf_test.txt

# node.js perf test
echo "----node.js perf test----" >> tmp/perf_test.txt
node wasm_perf_test.js >> tmp/perf_test.txt

# wavm perf test
echo "----WAVM perf test----" >> tmp/perf_test.txt
sizes=(32 64 128 256 512 1024 2048)
for size in ${sizes[@]}; do
    start=$(date +%s.%N)
    ../../WAVM/build/bin/wavm run --abi=bare --function=calculate perf_test.wasm $size
    end=$(date +%s.%N)
    duration=$(echo "$end - $start" | bc)
    echo "Size $size: $duration seconds" >> tmp/perf_test.txt
done

# wasmtime perf test
echo "----wasmtime perf test----" >> tmp/perf_test.txt
cargo run --bin wasmtime_perf_test >> tmp/perf_test.txt



echo "----wasm2c with -O0 flags----" >> tmp/perf_test.txt
gcc -O0 perf_test.c wasm2c_main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -lm \
    -o wasm2c_perf_test
./wasm2c_perf_test >> tmp/perf_test.txt

echo "----wasm2c with -O1 flags----" >> tmp/perf_test.txt
gcc -O1 perf_test.c wasm2c_main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -lm \
    -o wasm2c_perf_test
./wasm2c_perf_test >> tmp/perf_test.txt

echo "----wasm2c with -O2 flags----" >> tmp/perf_test.txt
gcc -O2 perf_test.c wasm2c_main.c \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-impl.o \
    ~/workspace/course_project/wabt/wasm2c/wasm-rt-mem-impl.o \
    -I~/workspace/course_project/wabt/wasm2c \
    -pthread \
    -lm \
    -o wasm2c_perf_test
./wasm2c_perf_test >> tmp/perf_test.txt