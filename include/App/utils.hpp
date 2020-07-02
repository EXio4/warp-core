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

// taken from https://helloacm.com/cc-function-to-compute-the-bilinear-interpolation/
inline double BilinearInterpolation(double q11, double q12, double q21, double q22, double x1, double x2, double y1, double y2, double x, double y) 
{
    double x2x1, y2y1, x2x, y2y, yy1, xx1;
    x2x1 = x2 - x1;
    y2y1 = y2 - y1;
    x2x = x2 - x;
    y2y = y2 - y;
    yy1 = y - y1;
    xx1 = x - x1;
    return 1.0 / (x2x1 * y2y1) * (
        q11 * x2x * y2y +
        q21 * xx1 * y2y +
        q12 * x2x * yy1 +
        q22 * xx1 * yy1
    );
}