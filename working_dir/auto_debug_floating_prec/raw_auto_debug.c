#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE

int calculate(float i) {
    float a = i + i + i + i + i + i + i + i + i + i;
    float b = i*10;
    if (a == b) {
        return 1;
    }
    return 0;
}