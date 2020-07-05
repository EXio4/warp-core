#include <emscripten.h>
#include <emscripten/bind.h>

#include "App/Render.hpp"

Render _r;

void initRender() {
    _r.startThread();
}
void updateCanvas(int width, int height) {
    _r.updateCanvas(width, height);
}


/* this code returns a pointer so embind becomes annoying */
extern "C" {

    uint32_t* EMSCRIPTEN_KEEPALIVE render() {
        return _r.getRenderedFrame();
    }

};

EMSCRIPTEN_BINDINGS(render) {
    emscripten::function("initRender", &initRender);
    emscripten::function("updateCanvas", &updateCanvas);
}
