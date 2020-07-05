#include "debug.hpp"

#include <emscripten/bind.h>

DebugInfo debug;

/* FIXME: learn how the fuck you're supposed to export nested classes while avoiding copies */
int debug_mapgen_count() {
    return debug.mapgen.getCount();
}
double debug_mapgen_averageTime() {
    return debug.mapgen.getAverageTime();
}
double debug_mapgen_lastTime() {
    return debug.mapgen.getLastTime();
}


int debug_render_count() {
    return debug.render.getCount();
}
double debug_render_lastTime() {
    return debug.render.getLastTime();
}
double debug_render_averageTime() {
    return debug.render.getAverageTime();
}

EMSCRIPTEN_BINDINGS(debug) {
    emscripten::function("debug_mapgen_count", &debug_mapgen_count);
    emscripten::function("debug_mapgen_lastTime", &debug_mapgen_lastTime);
    emscripten::function("debug_mapgen_averageTime", &debug_mapgen_averageTime);
    emscripten::function("debug_render_count", &debug_render_count);
    emscripten::function("debug_render_lastTime", &debug_render_lastTime);
    emscripten::function("debug_render_averageTime", &debug_render_averageTime);
}
