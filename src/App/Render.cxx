
#include <cmath>

#include "App/Render.h"
#include "App/utils.h"

Render::Render() {
    width = 0;
    height = 0;
    data = 0;
}

void Render::updateCanvas(int cWidth, int cHeight) {
  width = cWidth;
  height = cHeight;
  int pixelCount = width * height;
  if (data != 0) {
    delete[] data;
  }
  data = new int[pixelCount];
}

int* Render::render(double timestamp) {

    for (int y = 0; y < height; y++) {
        int yw = y * width;
        for (int x = 0; x < width; x++) {
            int factor = (sin((timestamp/10 + x)/20) + 1) * 127;
            data[yw + x] = rgba(factor, 0, factor, 255);
        }
    }

    return data;
}