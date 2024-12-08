#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE

int calculate(int i) {
    for (int j = 0; j < i; j++) {
        int a = i;
    }
    if (i % 2 == 0) {
        return *(&i+1);
    }
    return i;
}