#pragma once
#include <cstdint>

struct Camera {
    double x;
    double y;
    double height;
    double angle;
    double horizon;
    uint32_t distance;

    Camera(double x, double y, double height, double angle, double horizon, uint32_t distance) :
        x(x), y(y), height(height), angle(angle),
        horizon(horizon), distance(distance) {};
    Camera() :
        x(0), y(0), height(100), angle(0), horizon(100), distance(400) {};
};