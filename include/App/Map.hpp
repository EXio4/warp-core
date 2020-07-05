#pragma once

#include <atomic>
#include <thread>
#include <shared_mutex>
#include <cstdint>
#include <utility> // for std::pair
#include <map>
#include "Ext/PerlinNoise.hpp"

#define OCTAVES 8
#define CHUNK_SIZE 256

#define HEIGHT_RATIO 200
#define HUMID_RATIO 2048
#define TEMP_RATIO 1024

// implement this
enum TileBehavior {
    TVoid,
    TReflectiveLiquid,
    TLiquid,
    TTransparentSolid,
    TSolid,
};

enum TileBiome {
    BHell,
    BDesert,
    BIce,
    BNormal,
};

enum TileType {
    Lava,
    Water,
    Ice,
    Snow,
    Dirt,
    Sand,
    DesertSand,
    Grass,
    Stone,
    VolcanicRock,
};

struct TileData {
    TileBiome biome;
    TileBehavior behavior;
    TileType type;
    uint32_t color;
    uint32_t height;
    double light;
};

using Vector2D = std::pair<int32_t, int32_t>;

struct MapChunk {
    TileData* data;

    MapChunk() {
        data = NULL;
    }
    void initialize() {
        data = new TileData[CHUNK_SIZE*CHUNK_SIZE];
    }
    void deleteMemory() {
        if (data) delete[] data;
    }
    TileData rawGet(uint32_t offsetX, uint32_t offsetY);
    TileData get(uint32_t offsetX, uint32_t offsetY, double x, double y);
};

class Map {
public:
    Map();
    Map(uint32_t seed);
    ~Map() {
        keepRunning = false;
        mapgenThread.join();
        syncThread.join();
    }

    TileData get(double x, double y);

    void setCameraPosition(int x, int y);

    void startThreads();
private:

    bool keepRunning;

    double baselineBrightness;
    double sunlightBrightness;
    double sunlightAngle;

    void genTile(TileData& res, double x, double y, bool calculateLight);
    void mapgen(MapChunk& chunk, int32_t chunkX, int32_t chunkY);

    void mapgenLoop();
    void syncLoop();

    std::thread mapgenThread;
    std::thread syncThread;

    std::atomic<bool> updatedCamera;
    mutable std::shared_mutex camera_mutex_;
    int cameraX;
    int cameraY;


    mutable std::shared_mutex render_mutex_;
    std::map<Vector2D, MapChunk> *renderMap;
    mutable std::shared_mutex mapgen_mutex_;
    std::map<Vector2D, MapChunk> *mapgenMap;

    siv::PerlinNoise heightNoise;
    siv::PerlinNoise humidNoise;
    siv::PerlinNoise tempNoise;
    siv::PerlinNoise extraNoise;

};
