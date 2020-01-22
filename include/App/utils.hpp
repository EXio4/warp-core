#pragma once
#include <cstdlib>
#include <cstdint>

uint32_t rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255);

// https://stackoverflow.com/questions/828092/python-style-integer-division-modulus-in-c
static div_t div_floor(int x, int y) {
    div_t r = div(x, y);
    if (r.rem && (x < 0) != (y < 0)) {
        r.quot -= 1;
        r.rem  += y;
    }
    return r;
}