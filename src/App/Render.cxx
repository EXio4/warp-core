
#include <cmath>

#include "input.h"
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
            int factor1 = (sin((timestamp/10 + x)/14) + 1) * 127;
            int factor2 = 0; // (sin((timestamp/10 + y)/20) + 1) * 127;
            int factor3 = input.up > 0 ? (sin((timestamp/10 + y)/23) + 1) * 127 : 0;
            data[yw + x] = rgba(factor1, factor2, factor3, 255);
        }
    }

    return data;
}