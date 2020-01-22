
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
        30.00,  // angle
        180.58, // horizon
        400     // distance
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

uint32_t applyLight (uint32_t color, double light) {
    return color;
    // uint32_t r = (color & 0xff000000) >> 24;
    // uint32_t g = (color & 0x00ff0000) >> 16;
    // uint32_t b = (color & 0x0000ff00) >>  8;
    // printf("%d, %d, %d, %d\n", color, r, g, b);
    // r = (r * light) / 100;
    // g = (g * light) / 100;
    // b = (b * light) / 100;
    // return rgba(r, g, b, 0xff);
}

uint32_t* Render::render(double timestamp) {
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
        double dx = (prx - plx) / (0.0 + width);
        double dy = (pry - ply) / (0.0 + width);
        plx += camera.x;
        ply += camera.y;
        double invz = 1 / z * 240;

        for (uint32_t i=0; i<width; i++) {
            TileData tile = map.get(plx, ply);
            double height = (camera.height - tile.height) * invz + camera.horizon;
            drawVLine(i, height, hiddeny[i], applyLight(tile.color, tile.light));
            if (height < hiddeny[i]) hiddeny[i] = height;
            plx += dx;
            plx += dy;
            deltaz += 0.005;
        }
    }

    return data;
}

/*
let sinang = Math.sin(camera.angle);
        let cosang = Math.cos(camera.angle);
    
        let hiddeny = new uint32_t32Array(screenwidth);
        for(let i=0; i<screendata.current.canvas.width|0; i=i+1|0)
            hiddeny[i] = screendata.current.canvas.height;
    
        let deltaz = 1.;
    
        // Draw from front to back
        for(let z=1; z<camera.distance; z+=deltaz)
        {

            // 90 degree field of view
            let plx =  -cosang * z - sinang * z;
            let ply =   sinang * z - cosang * z;
            let prx =   cosang * z - sinang * z;
            let pry =  -sinang * z - cosang * z;
    
            let dx = (prx - plx) / screenwidth;
            let dy = (pry - ply) / screenwidth;
            plx += camera.x;
            ply += camera.y;
            let invz = 1. / z * 240.;
            for(let i=0; i<screenwidth|0; i=i+1|0)
            {
                const data = info.current.getData(plx, ply)
                // type is meant to be used for:
                // - animations
                // - things
                // no idea really, actually, seems useful later
                const heightonscreen = (camera.height - data.altitude) * invz + camera.horizon|0;
                voxelDrawLine(i, heightonscreen|0, hiddeny[i], data.color);
                if (heightonscreen < hiddeny[i]) hiddeny[i] = heightonscreen;
                plx += dx;
                ply += dy;
            }
            deltaz += 0.005;

        }
*/