#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE

int calculate(int i) {
    int *p = (int *)malloc(sizeof(int));
    *p = i;
    free(p);
    return *p;
}