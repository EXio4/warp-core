#include <emscripten.h>
#include <App/Render.hpp>

Render _r;

extern "C" {     
    void EMSCRIPTEN_KEEPALIVE initRender() {
        _r.startThread();
    }
    void EMSCRIPTEN_KEEPALIVE updateCanvas(int width, int height) {
        _r.updateCanvas(width, height);
    }
    uint32_t* EMSCRIPTEN_KEEPALIVE render() {
        return _r.getRenderedFrame();
    }
}