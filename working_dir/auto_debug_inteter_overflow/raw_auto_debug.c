#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE

int calculate(int i) {
    if ((i > 0 && i*2 > 0) || (i < 0 && i*2 < 0)) {
        return 0;
    }
    return 1;
}
