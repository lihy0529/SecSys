#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE

int calculate(int i) {
    int a[5] = {};
    a[5] = 1919180/114514;
    return a[i];
}