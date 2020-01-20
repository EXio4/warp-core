#include <emscripten.h>

extern "C" {     
    void EMSCRIPTEN_KEEPALIVE initRender();
    void EMSCRIPTEN_KEEPALIVE updateCanvas(int width, int height);
    int* EMSCRIPTEN_KEEPALIVE render(double timestamp);
}