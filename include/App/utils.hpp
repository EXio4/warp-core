#pragma once
#include <cstdint>

uint32_t rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255);

template <typename T>
T mod(T k, T n) {
    return ((k %= n) < 0) ? k+n : k;
}