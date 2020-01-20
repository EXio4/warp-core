#include <emscripten.h>
#include <App/Render.h>

Render _r;

extern "C" {     
    void EMSCRIPTEN_KEEPALIVE initRender() {
    }
    void EMSCRIPTEN_KEEPALIVE updateCanvas(int width, int height) {
        _r.updateCanvas(width, height);
    }
    int* EMSCRIPTEN_KEEPALIVE render(double timestamp) {
        return _r.render(timestamp);
    }
}