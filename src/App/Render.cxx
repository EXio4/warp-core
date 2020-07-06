
#include <cstdint>
#include <vector>
#include <cmath>
#include <cstring>

#include "App/Input.hpp"
#include "App/Render.hpp"
#include "App/utils.hpp"
#include "debug.hpp"

Render::Render() : map(1000), camera(
        206.88, // x
        192.58, // y
        180.96, // height
        M_PI/2,  // angle
        160.58, // horizon
        350     // distance
    ) {
    width = 0;
    height = 0;
    
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;


    renderBuffer = 0;
    lastBuffer = 1;
    readyBuffer = 2;

    lastFrame = std::chrono::steady_clock::now();
    keepRender = true;

    frameCount = 0;
    lastRender = -1;

    farDetail = 1;
    closeDetail = 1;
    overallDetail = 1;
}

Render::~Render() {
    keepRender = false;
    renderThread.join();
}

void Render::startThread() {
    map.startThreads();
    renderThread = std::thread(&Render::renderLoop, &*this);
}

void Render::updateCanvas(uint32_t cWidth, uint32_t cHeight) {
  width = cWidth;
  height = cHeight;
  uint32_t pixelCount = width * height;
  updatedDimensions = true;
  for (int i = 0; i < 3; i++) {
      if (buffer[i] != 0) {
          delete[] buffer[i];
      }
      buffer[i] = new uint32_t[pixelCount];
  }
}

void Render::renderSky(uint32_t* data, uint32_t _width, uint32_t _height) {
    uint32_t skyColor = rgba(135, 206, 235, 255);
    for (int i=0; i<_height*_width; i++) {
        data[i] = skyColor;
    }
}

uint32_t* Render::finishRender() {
    std::unique_lock<std::shared_mutex> lock(buffer_mutex_);
    int nextBuffer = lastBuffer;
    lastBuffer = renderBuffer;
    renderBuffer = nextBuffer;
    frameCount++;
    return buffer[renderBuffer];
}

uint32_t* Render::getRenderedFrame() {
    std::unique_lock<std::shared_mutex> lock(buffer_mutex_);
    if (frameCount == lastRender) {
        /* we haven't rendered a new frame since the last render, just output the last known frame to work */
        return buffer[readyBuffer];
    }
    lastRender = frameCount;
    int nextBuffer = lastBuffer;
    lastBuffer = readyBuffer;
    readyBuffer = nextBuffer;
    return buffer[readyBuffer];
}

void Render::drawVLine(uint32_t* data, uint32_t width, uint32_t x, uint32_t ytop, uint32_t ybottom, uint32_t color) {
    if (ytop < 0) ytop = 0;
    if (ytop > ybottom) return;

    uint32_t offset = ytop * width + x;
    for (uint32_t k=ytop; k<ybottom; k++) {
        data[offset] = color;
        offset += width;
    }
}

void Render::updateCamera(std::chrono::time_point<std::chrono::steady_clock>& currentFrame) {
    const std::chrono::duration<double, std::milli> deltaFrames = currentFrame - lastFrame;

    int leftright = input.left - input.right;
    int forwardbackward = 3 * (input.up - input.down);
    int updown = 2 * (input.lookup - input.lookdown);
    double deltaTime =  0.03 * 1.2 * deltaFrames.count();
    if (leftright != 0) {
        camera.angle += leftright * 0.1 * deltaTime;
    }
    if (forwardbackward != 0) {
        camera.x -= forwardbackward * sin(camera.angle) * deltaTime;
        camera.y -= forwardbackward * cos(camera.angle) * deltaTime;
    }
    if (updown != 0) {
      camera.height += updown * deltaTime;
    }

}

uint32_t Render::applyEffects (uint32_t color, double light, double distanceRatio) {
    uint32_t r = (color & 0x000000ff);
    uint32_t g = (color & 0x0000ff00) >>  8;
    uint32_t b = (color & 0x00ff0000) >> 16;
    // sky color
    uint32_t skyR = 135;
    uint32_t skyG = 206;
    uint32_t skyB = 235;
    r = (r * light) / 100;
    g = (g * light) / 100;
    b = (b * light) / 100;
    if (distanceRatio < 0.05) {
        return rgba(r, g, b, 255);
    } else {
        r = r * (1 - distanceRatio) + skyR * distanceRatio;
        g = g * (1 - distanceRatio) + skyG * distanceRatio;
        b = b * (1 - distanceRatio) + skyB * distanceRatio;
        return rgba(r, g, b, 255);
    }
}


void Render::workerLoop(std::shared_ptr<MVar<WorkerCommand>> inputChan, std::shared_ptr<MVar<RenderCommand>> renderChan) {
    std::vector<uint32_t> hiddeny(width, height);
    int thNumber = -1;
    while (true) {
        std::unique_ptr<WorkerCommand> cmd = inputChan->read();

        if (cmd->type == QuitThread) {
            return;
        }
        if (cmd->type == StartRendering) {
            uint32_t* data = cmd->data.startRendering.buffer;
            uint32_t* fullBuffer = cmd->data.startRendering.fullBuffer;
            RenderConfig cfg = cmd->data.startRendering.cfg;
            thNumber = cfg.threadNumber;
            const int properWidth = cfg.endWidth - cfg.startWidth;
            hiddeny.clear();
            hiddeny.resize(properWidth, height);
            double sinang = sin(camera.angle);
            double cosang = cos(camera.angle);

            double deltaz = 0.25;
            renderSky(data, properWidth, height);

            for (double z=1; z<camera.distance; z+=deltaz) {
                double plx =  -cosang * z - sinang * z;
                double ply =   sinang * z - cosang * z;
                double prx =   cosang * z - sinang * z;
                double pry =  -sinang * z - cosang * z;

                double dx = (prx - plx) / width;
                double dy = (pry - ply) / width;

                plx += camera.x;
                ply += camera.y;

                double invz = (1 / z) * 240;
                double dR = z / camera.distance;
                double fogRatio;
                if (dR < 0.5) {
                    fogRatio = 0;
                } else {
                    fogRatio = (dR - 0.5) * (1/0.5);
                }

                plx += dx * cfg.startWidth;
                ply += dy * cfg.startWidth;
                for (uint32_t i=0; i<properWidth; i++) {
                    TileData tile = map.get(plx, ply);
                    double height = (camera.height - (double)tile.height) * invz + camera.horizon;
                    drawVLine(data, properWidth, i, height, hiddeny[i], applyEffects(tile.color , tile.light, fogRatio));
                    if (height < hiddeny[i]) hiddeny[i] = height;
                    plx += dx;
                    ply += dy;
                }
                
                deltaz = overallDetail * (
                        (0.2 + 0.4 * dR + 0.4 * dR * dR) * closeDetail + // close-up formula
                        (-0.4375 * dR + 3.4375 * dR * dR) * farDetail // far-away formula
                    );
//                deltaz = 2 * dR * dR + 0.5 * dR + 0.25;
            }

            for (int k=0; k < height; k++) {
                uint32_t offsetInternal = k * properWidth;
                uint32_t offsetLocal = k * width + cfg.threadNumber * properWidth;
                std::memcpy(&fullBuffer[offsetLocal], &data[offsetInternal], properWidth * sizeof(uint32_t));
            }       

            std::unique_ptr<RenderCommand> finished = std::make_unique<RenderCommand>();
            finished->type = FinishedRendering;
            renderChan->write(std::move(finished));
        }
    }
}

struct RenderThreadInfo {
    std::thread th;
    std::shared_ptr<MVar<WorkerCommand>> workerChan;
    std::shared_ptr<MVar<RenderCommand>> renderChan;
    uint32_t* buffer;
    RenderConfig cfg;
    RenderThreadInfo() : buffer(NULL) {};
};

void Render::renderLoop() {

    int targetFPS = 35;
    const uint8_t threadCount = 4;
    double thWidth = width;
    std::vector<RenderThreadInfo> threads;

    threads.resize(threadCount);
    for (int i=0; i < threadCount; i++) {
        threads[i].workerChan = std::make_shared<MVar<WorkerCommand>>();
        threads[i].renderChan = std::make_shared<MVar<RenderCommand>>();
        threads[i].cfg.threadNumber = i;
        threads[i].th = std::thread(&Render::workerLoop, &*this, threads[i].workerChan, threads[i].renderChan);
    }

    const auto& onResize = [&]() {
        thWidth = width / threadCount;
        for (int i=0; i < threadCount; i++) {
            threads[i].cfg.startWidth = i * thWidth;
            threads[i].cfg.endWidth = (i+1) * thWidth;
            if (threads[i].buffer) {
                delete[] threads[i].buffer;
            }
            threads[i].buffer = new uint32_t[(int)thWidth * height];
        }
    };

    onResize();


    double runningAvgFPS = 0;

    while (keepRender) {
        uint32_t* data = finishRender();
        std::chrono::time_point<std::chrono::steady_clock> renderStartTime = std::chrono::steady_clock::now();
    
        updateCamera(renderStartTime);
        map.setCameraPosition(camera.x, camera.y); 

        if (updatedDimensions) {
            updatedDimensions = false;
            onResize();
        }

        for (int i=0; i<threadCount; i++) {
            std::unique_ptr<WorkerCommand> cmd = std::make_unique<WorkerCommand>();
            cmd->type = StartRendering;
            cmd->data.startRendering.buffer = threads[i].buffer;
            cmd->data.startRendering.fullBuffer = data;
            cmd->data.startRendering.cfg = threads[i].cfg;

            threads[i].workerChan->write(std::move(cmd));
        }

        for (int i=0; i<threadCount; i++) {
            std::unique_ptr<RenderCommand> cmd = threads[i].renderChan->read();
            if (cmd->type != FinishedRendering) {
                std::cout << "Error while rendering" << std::endl;
            }
        }
        
        std::chrono::time_point<std::chrono::steady_clock> renderFinalTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double, std::milli> renderTime = renderFinalTime - renderStartTime;
        debug.render.newEntry(renderTime.count());

        /* dynamic view distance scaling */

        double fps = 1000 / renderTime.count();
        runningAvgFPS = fps * 0.75 + runningAvgFPS * (1-0.75);
        if (runningAvgFPS < targetFPS * 0.75 && overallDetail <= 4) {
            overallDetail *= 1.001;
            farDetail *= 1.001;
            closeDetail *= 1.005;
        }
        if (runningAvgFPS > targetFPS * 1.25) {
            overallDetail *= 0.999;
            farDetail *= 0.98;
            closeDetail *= 0.95;
        }

        lastFrame = renderStartTime;
    }
}