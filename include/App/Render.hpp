#pragma once

#include <thread>
#include <shared_mutex>
#include <chrono>
#include <cstdint>
#include "App/Camera.hpp"
#include "App/Map.hpp"
#include "Ext/MVar.hpp"


enum WorkerCommandType {
    StartRendering,
    QuitThread,
};

struct RenderConfig {
    int startWidth;
    int endWidth;
    int threadNumber;
};

union WorkerCommandData {
    struct {
        uint32_t* buffer;
        RenderConfig cfg;
    } startRendering;
};

struct WorkerCommand {
    WorkerCommandType type;
    WorkerCommandData data;
};

enum RenderCommandType {
    FinishedRendering,
};

struct RenderCommand {
    RenderCommandType type;
};


class Render {
public:
    Render();
    ~Render();
    void startThread();
    void updateCanvas(uint32_t width, uint32_t height);
    uint32_t* getRenderedFrame();    
private:

    std::thread renderThread;

    void workerLoop(std::shared_ptr<MVar<WorkerCommand>> inputChan, std::shared_ptr<MVar<RenderCommand>> renderChan);

    std::chrono::time_point<std::chrono::steady_clock> lastFrame;

    void updateCamera(std::chrono::time_point<std::chrono::steady_clock>& currentFrame);
    void renderSky(uint32_t* data, int startWidth, int endWidth, int height);
    void drawVLine(uint32_t* data, uint32_t x, uint32_t ytop, uint32_t ybottom, uint32_t color);

    void renderLoop();

    uint32_t applyEffects (uint32_t color, double light, double distanceRatio);

    Map map;
    Camera camera;

    uint32_t width;
    uint32_t height;

    mutable std::shared_mutex buffer_mutex_;
    uint32_t* buffer[3];
    uint8_t renderBuffer;
    uint8_t readyBuffer;
    uint8_t lastBuffer;
    
    uint32_t frameCount;
    uint32_t lastRender;

    uint32_t* finishRender();

    std::atomic<bool> updatedDimensions;
    bool keepRender;
};