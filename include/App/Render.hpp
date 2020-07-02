#pragma once

#include <thread>
#include <shared_mutex>
#include <chrono>
#include <cstdint>
#include "App/Camera.hpp"
#include "App/Map.hpp"

class Render {
public:
    Render();
    ~Render();
    void startThread();
    void updateCanvas(uint32_t width, uint32_t height);
    uint32_t* getRenderedFrame();    
private:

    std::thread renderThread;
    std::chrono::time_point<std::chrono::steady_clock> lastFrame;

    void updateCamera();
    void renderSky(uint32_t* data);
    void drawVLine(uint32_t* data, uint32_t x, uint32_t ytop, uint32_t ybottom, uint32_t color);

    void renderLoop();

    uint32_t applyEffects (uint32_t color, double light, double distanceRatio);

    Map map;
    Camera camera;

    uint32_t width;
    uint32_t height;

    mutable std::shared_mutex buffer_mutex_;
    uint32_t* buffer[3];
    int renderBuffer;
    int readyBuffer;
    int lastBuffer;

    uint32_t* finishRender();


    bool keepRender;
};