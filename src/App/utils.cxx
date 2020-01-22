#include "App/utils.hpp"

uint32_t rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
    return
        (r << 24) |  // R
        (g << 16) |  // G
        (b <<  8) |  // B
         a;          // A
}