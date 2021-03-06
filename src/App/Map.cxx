#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdint>

#include "App/Map.hpp"
#include "App/utils.hpp"

#include "debug.hpp"

TileData voidTile = {
    BNormal,
    TVoid,
    Stone,
    rgba(0,0,0, 0xff),
    0,
    0,
};

uint32_t rgb_with_noise(double colorNoise, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t _r = std::min(255.0, ((double)r) * (1 + colorNoise));
    uint8_t _g = std::min(255.0, ((double)g) * (1 + colorNoise));
    uint8_t _b = std::min(255.0, ((double)b) * (1 + colorNoise));
    return rgba(_r,_g,_b, 0xff);
}

uint32_t getColor(TileType type, double colorNoise) {
    switch (type) {
        case Grass:
            return rgb_with_noise(colorNoise, 0x98, 0xdd, 0x00);
        case Water:
            return rgb_with_noise(colorNoise, 0x00, 0xdd, 0xca);
        case Sand:
            return rgb_with_noise(colorNoise, 0xc2, 0xb2, 0x80);
        case DesertSand:
            return rgb_with_noise(colorNoise, 0xf4, 0xa4, 0x60);
        case Dirt:
            return rgb_with_noise(colorNoise, 0xbb, 0x8b, 0x00);
        case Stone:
            return rgb_with_noise(colorNoise, 0x8d, 0x8d, 0x8d);
        case Ice:
            return rgb_with_noise(colorNoise, 0xb9, 0xe8, 0xea);
        case Snow:
            return rgb_with_noise(colorNoise, 0xff, 0xfa, 0xfa);
        case Lava:
            return rgb_with_noise(colorNoise, 0xcf, 0x10, 0x20);
        case VolcanicRock:
            return rgb_with_noise(colorNoise, 0x3d, 0x3f, 0x3f);
        default:
            return rgba(0, 0, 0, 0xff);
    }
}

Map::Map() : renderMap(new std::map<Vector2D, MapChunk*>()), mapgenMap(new std::map<Vector2D, MapChunk*>()) {
    keepRunning = true;
    baselineBrightness = 20;
    sunlightBrightness = 100;
    sunlightAngle = M_PI/3;
    cameraX = cameraY = 0;
}

Map::Map(uint32_t seed) : renderMap(new std::map<Vector2D, MapChunk*>()), mapgenMap(new std::map<Vector2D, MapChunk*>()), heightNoise(seed), tempNoise(seed + 2), humidNoise(seed + 3), extraNoise(seed + 4) {
    keepRunning = true;
    baselineBrightness = 20;
    sunlightBrightness = 100;
    sunlightAngle = M_PI/3;
    cameraX = cameraY = 0;
}


void Map::setCameraPosition(int x, int y) {
    int chunkX = x/CHUNK_SIZE;
    int chunkY = y/CHUNK_SIZE;
    
    std::unique_lock<std::shared_mutex> lock(camera_mutex_);
    if (abs(cameraX - chunkX) >= 2 || abs(cameraY - chunkY) >= 2) {
        cameraX = chunkX;
        cameraY = chunkY;
        updatedCamera = true;
    }
}

void Map::genTile(TileData& res, double x, double y, bool calculateLight) {
    TileBiome biome = BNormal;
    TileBehavior behavior = TSolid;
    TileType type = Water;

    double noise = 16 * (heightNoise.octaveNoise(x/HEIGHT_RATIO, y/HEIGHT_RATIO, OCTAVES) + 1) - 1;
    double humid = 50 * (humidNoise.octaveNoise(x/HUMID_RATIO, y/HUMID_RATIO, OCTAVES) + 1);
    double temp  = 32.5 * (tempNoise.octaveNoise(x/TEMP_RATIO, y/TEMP_RATIO, OCTAVES) + 1) - 15;
    double colorNoise = (50 * (extraNoise.octaveNoise(10 * x/HEIGHT_RATIO, 10 * y/HEIGHT_RATIO, OCTAVES) + 1) - 25) / 200;

    // range for values:
    // noise = 0-32
    // humid = 0 to 100
    // temp = -15 to 50
    if (temp <= 0) {
        biome = BIce;
        // icy 
        if (noise <= 6) {
            type = Ice;
            noise = 5;
        } else {
            type = Snow;
        }
    } else if (temp <= 40) {
        biome = BNormal;
        // normal
        if (humid <= 25 && noise <= 7) {
            // adjust water level due to humidity
            noise += 5 * (25-humid)/25;
        }
        if (noise <= 6) {
            if (temp <= 2.5) {
                biome = BIce;
                type = Ice;
            } else if (temp <= 38) {
                type = Water;
                behavior = TReflectiveLiquid;
            } else {
                biome = BHell;
                type = Lava;
                behavior = TLiquid;
            }
            noise = 5;
        } else if (humid <= 25) {
            biome = BDesert;
            type = DesertSand;
        } else if (noise <= 15) {
            biome = BDesert;
            type = Sand;
        } else if (noise <= 25) {
            type = Grass;
        } else {
            type = Stone;
        }
    } else {
        biome = BHell;
        // hell-ish
        if (noise <= 6) {
            type = Lava;
            noise = 5;
            behavior = TLiquid;
        } else {
            type = VolcanicRock;
        }
    }

    int32_t height = noise * 6;
    double light = sunlightBrightness;
    uint32_t color = getColor(type, colorNoise);

    if (calculateLight) {
        TileData temp;
        double maxDistance = 42/sin(sunlightAngle * M_PI/180);
        // cap the shadow stuff at this distance
        if (maxDistance > 30) maxDistance = 30;
        for (double dy = 0; dy < maxDistance; dy += 1) {
            genTile(temp, x, y + dy, false);
            double delta = maxDistance;
            if (temp.height > height) {
                light = baselineBrightness + (sunlightBrightness - baselineBrightness) * (1 - (delta - dy) / delta);
                // light = (sunlightBrightness - baselineBrightness) * ((delta - dy) / delta);
                break;
            }
        }
    }

    res.biome = biome;
    res.behavior = behavior;
    res.type = type;
    res.color = color;
    res.height = height;
    res.light = light;
}
void Map::mapgen(MapChunk* chunk, int32_t chunkX, int32_t chunkY) {
    
    std::chrono::time_point<std::chrono::steady_clock> mapgenStartTime = std::chrono::steady_clock::now();
    for (int32_t dy = 0; dy < CHUNK_SIZE; dy++) {
        for (int32_t dx = 0; dx < CHUNK_SIZE; dx++) {
            double x = chunkX * CHUNK_SIZE + dx;
            double y = chunkY * CHUNK_SIZE + dy;
            genTile(chunk->data[dx * CHUNK_SIZE + dy], x, y, true);
        }
    }
    std::chrono::time_point<std::chrono::steady_clock> mapgenEndTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> mapgenTime = mapgenEndTime - mapgenStartTime;
    debug.mapgen.newEntry(mapgenTime.count());
}

void Map::startThreads() {
    mapgenThread = std::thread(&Map::mapgenLoop, &*this);
    syncThread = std::thread(&Map::syncLoop, &*this);
}

void Map::syncLoop() {
    using namespace std::chrono_literals;
    while (keepRunning) {
        std::this_thread::sleep_for(1500ms);
        
        std::unique_lock<std::shared_mutex> lock2(mapgen_mutex_);
        mapgenMap = renderMap.exchange(mapgenMap);
        *mapgenMap = *renderMap;

    }
}

void Map::mapgenLoop() {    
    int x = cameraX; // current position; x
    int y = cameraY; // current position; y
    newIteration:
    
    while (keepRunning) {
            int d = 0; // current direction; 0=RIGHT, 1=DOWN, 2=LEFT, 3=UP
            int c = 0; // counter
            int s = 1; // chain size
            int size = 5;
            // code from https://stackoverflow.com/questions/33684970/print-2-d-array-in-clockwise-expanding-spiral-from-center
            for (int k=1; k<=(size-1); k++)
            {
                for (int j=0; j<(k<(size-1)?2:3); j++)
                {
                    for (int i=0; i<s; i++)
                    {
                        c++;
                        if (updatedCamera.load(std::memory_order_relaxed)) {
                            std::unique_lock<std::shared_mutex> lock(camera_mutex_);
                            x = cameraX;
                            y = cameraY;
                            updatedCamera = false;
                            lock.unlock();
                            goto newIteration;
                        }

                        Vector2D pos = std::make_pair(x, y);
                        std::unique_lock<std::shared_mutex> mapgenLock(mapgen_mutex_);
                        const auto& chunkIter = mapgenMap->find(pos);
                        bool ret = chunkIter == mapgenMap->end();
                        mapgenLock.unlock();
                        
                        if (ret) {
                            MapChunk* chunk = new MapChunk;
                            mapgen(chunk, x, y);
                            {
                                std::unique_lock<std::shared_mutex> lock(mapgen_mutex_);
                                mapgenMap->insert({ pos, chunk });
                            }
                        }

                        switch (d)
                        {
                            case 0: y = y + 1; break;
                            case 1: x = x + 1; break;
                            case 2: y = y - 1; break;
                            case 3: x = x - 1; break;
                        }
                    }
                    d = (d+1)%4;
                }
                s = s + 1;
            }
    }
}

TileData MapChunk::rawGet(uint32_t offsetX, uint32_t offsetY) {
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    if (offsetX >= CHUNK_SIZE) offsetX = CHUNK_SIZE - 1;
    if (offsetY >= CHUNK_SIZE) offsetY = CHUNK_SIZE - 1;
    return data[offsetX * CHUNK_SIZE + offsetY];
}

TileData MapChunk::get(uint32_t offsetX, uint32_t offsetY, double x, double y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (offsetX > x) {
        offsetX --;

    }
    if (offsetY > y) {
        offsetY --;
    }
    TileData t = rawGet(offsetX, offsetY);
    TileData t_y = rawGet(offsetX, offsetY + 1);
    TileData t_x = rawGet(offsetX + 1, offsetY);
    TileData t_xy = rawGet(offsetX + 1, offsetY + 1);

    TileData ret;
    ret.biome = t.biome;
    ret.behavior = t.behavior;
    ret.type = t.type;
    
    uint32_t r_0 = (t.color & 0x000000ff);
    uint32_t g_0 = (t.color & 0x0000ff00) >>  8;
    uint32_t b_0 = (t.color & 0x00ff0000) >> 16;

    
    uint32_t r_x = (t_x.color & 0x000000ff);
    uint32_t g_x = (t_x.color & 0x0000ff00) >>  8;
    uint32_t b_x = (t_x.color & 0x00ff0000) >> 16;

    
    uint32_t r_y = (t_y.color & 0x000000ff);
    uint32_t g_y = (t_y.color & 0x0000ff00) >>  8;
    uint32_t b_y = (t_y.color & 0x00ff0000) >> 16;

    
    uint32_t r_xy = (t_xy.color & 0x000000ff);
    uint32_t g_xy = (t_xy.color & 0x0000ff00) >>  8;
    uint32_t b_xy = (t_xy.color & 0x00ff0000) >> 16;


    if (t.behavior != t_y.behavior || t.behavior != t_x.behavior || t.behavior != t_xy.behavior) {
        ret.color = t.color;
        ret.height = t.height;
    } else {
        ret.color = rgba(
            BilinearInterpolation(r_0, r_y, r_x, r_xy, offsetX, offsetY, offsetX+1, offsetY+1, x, y),
            BilinearInterpolation(g_0, g_y, g_x, g_xy, offsetX, offsetY, offsetX+1, offsetY+1, x, y),
            BilinearInterpolation(b_0, b_y, b_x, b_xy, offsetX, offsetY, offsetX+1, offsetY+1, x, y),
            0xff
        );    
        ret.height = BilinearInterpolation(t.height, t_y.height, t_x.height, t_xy.height, offsetX, offsetY, offsetX+1, offsetY+1, x, y);
    }
    ret.light = BilinearInterpolation(t.light, t_y.light, t_x.light, t_xy.light, offsetX, offsetY, offsetX+1, offsetY+1, x, y);
    return ret;
}

TileData Map::get(double _x, double _y) {
    const div_t& pX = div_floor(_x, CHUNK_SIZE);
    const div_t& pY = div_floor(_y, CHUNK_SIZE);

    int32_t chunkX = pX.quot;
    int32_t chunkY = pY.quot;
    uint32_t offsetX = pX.rem;
    uint32_t offsetY = pY.rem;
    double px = _x - chunkX * CHUNK_SIZE;
    double py = _y - chunkY * CHUNK_SIZE;
    
    std::map<Vector2D, MapChunk*>& map = *renderMap.load(std::memory_order_relaxed);
    const auto& chunkIter = map.find({ chunkX, chunkY });
    if (chunkIter != map.end()) {
        return chunkIter->second->get(offsetX, offsetY, px, py);
    } else {
        return voidTile;
    }
}