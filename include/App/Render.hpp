#pragma once

#include <cstdint>
#include "App/Camera.hpp"
#include "App/Map.hpp"

class Render {
public:
    Render();
    void updateCanvas(uint32_t width, uint32_t height);
    uint32_t* render(double timestamp);    
private:

    void renderSky();
    void drawVLine(uint32_t x, uint32_t ytop, uint32_t ybottom, uint32_t color);

    Map map;
    Camera camera;

    uint32_t width;
    uint32_t height;
    uint32_t* data;
};