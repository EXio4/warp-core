#include <emscripten.h>
#include <emscripten/bind.h>
#include "App/Input.hpp"

InputState input;

void initInput() {
    input.left = input.right = input.up = input.down = input.mouseClick = 0;
    input.lookup = input.lookdown = 0;
    input.x = input.y = -1;
}

void  processKey(int key, int state) {
    switch (key) {
        case 87: // W
        case 38: // ArrowUp
            input.up = state;
        break;
        case 83: // S
        case 40: // ArrowDown
            input.down = state;
        break;
        case 65: // A
        case 37: // ArrowLeft
            input.left = state;
        break;
        case 68: // D
        case 39: // ArrowRight
            input.right = state;
        break;
        case 81: // Q
            input.lookup = state;
        break;
        case 69: // E
            input.lookdown = state;
        break;


    }
}

void  processMouseMovement(int x, int y) {
}

EMSCRIPTEN_BINDINGS(input) {
    emscripten::function("initInput", &initInput);
    emscripten::function("processKey", &processKey);
    emscripten::function("processMouseMovement", &processMouseMovement);
}
