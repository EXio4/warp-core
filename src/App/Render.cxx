
#include <cstdint>
#include <vector>
#include <cmath>

#include "input.hpp"
#include "App/Render.hpp"
#include "App/utils.hpp"

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
    data = 0;
}

void Render::updateCanvas(uint32_t cWidth, uint32_t cHeight) {
  width = cWidth;
  height = cHeight;
  uint32_t pixelCount = width * height;
  if (data != 0) {
    delete[] data;
  }
  data = new uint32_t[pixelCount];
}

void Render::renderSky() {
    uint32_t skyColor = rgba(135, 206, 235, 255);
    for (uint32_t i=0; i<width*height; i++) {
        data[i] = skyColor;
    }
}

void Render::drawVLine(uint32_t x, uint32_t ytop, uint32_t ybottom, uint32_t color) {
    if (ytop < 0) ytop = 0;
    if (ytop > ybottom) return;

    uint32_t offset = ytop * width + x;
    for (uint32_t k=ytop; k<ybottom; k++) {
        data[offset] = color;
        offset += width;
    }
}

void Render::updateCamera(double timestamp) {
    if (old_timestamp < 0) {
        old_timestamp = timestamp;
        return;
    }

    int leftright = input.left - input.right;
    int forwardbackward = 3 * (input.up - input.down);
    int updown = 2 * (input.lookup - input.lookdown);
    double deltaTime = 1.2 * (timestamp - old_timestamp);
    if (leftright != 0) {
        camera.angle += leftright * 0.1 * deltaTime * 0.03;
    }
    if (forwardbackward != 0) {
        camera.x -= forwardbackward * sin(camera.angle) * deltaTime * 0.03;
        camera.y -= forwardbackward * cos(camera.angle) * deltaTime * 0.03;
    }
    if (updown != 0) {
      camera.height += updown * deltaTime * 0.03;
    }
    old_timestamp = timestamp;
    /*
      const newTime = new Date().getTime()
  const newCamera = { ...state.camera }
  if (state.input.leftright !== 0) {
      newCamera.angle += state.input.leftright*0.1*(newTime-state.lastTime)*0.03;
  }
  if (state.input.forwardbackward !== 0) {
      newCamera.x -= state.input.forwardbackward * Math.sin(newCamera.angle) * (newTime-state.lastTime)*0.03;
      newCamera.y -= state.input.forwardbackward * Math.cos(newCamera.angle) * (newTime-state.lastTime)*0.03;
  }
  if (state.input.updown !== 0) {
      newCamera.height += state.input.updown * (newTime-state.lastTime)*0.03
  }
  if (state.input.look !== 0) {
    newCamera.horizon += state.input.look * (newTime-state.lastTime)*0.03;
  }
    */
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

uint32_t* Render::render(double timestamp) {
    updateCamera(timestamp);
    renderSky();

    double sinang = sin(camera.angle);
    double cosang = cos(camera.angle);

    std::vector<uint32_t> hiddeny(width, height);
    double deltaz = 1;
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
        double distanceRatio = z / camera.distance;
        if (distanceRatio < 0.25) {
            distanceRatio = 0;
        } else {
            distanceRatio = (distanceRatio - 0.4) * (1/0.6);
        }

        for (uint32_t i=0; i<width; i++) {
            TileData tile = map.get(plx, ply);
            double height = (camera.height - tile.height) * invz + camera.horizon;
            drawVLine(i, height, hiddeny[i], applyEffects(tile.color, tile.light, distanceRatio));
            if (height < hiddeny[i]) hiddeny[i] = height;
            plx += dx;
            ply += dy;
        }

        deltaz += 0.005;
    }

    return data;
}