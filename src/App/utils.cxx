#include "App/utils.hpp"

uint32_t rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
    return
        (a << 24) |  // A
        (b << 16) |  // B
        (g <<  8) |  // G
         r;          // R
}