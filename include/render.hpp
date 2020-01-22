#include <emscripten.h>

extern "C" {     
    void EMSCRIPTEN_KEEPALIVE initRender();
    void EMSCRIPTEN_KEEPALIVE updateCanvas(int width, int height);
    uint32_t* EMSCRIPTEN_KEEPALIVE render(double timestamp);
}