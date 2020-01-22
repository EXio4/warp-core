#pragma once

#include <cstdint>
#include <utility> // for std::pair
#include <map>
#include "Ext/PerlinNoise.hpp"

#define OCTAVES 8
#define CHUNK_SIZE 64

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
    bool initialized;
    TileData data[CHUNK_SIZE*CHUNK_SIZE];
};

class Map {
public:
    Map();
    Map(uint32_t seed);

    TileData get(double x, double y); 
private:

    double baselineBrightness;
    double sunlightBrightness;
    double sunlightAngle;

    void genTile(TileData& res, double x, double y, bool calculateLight);
    void mapgen(MapChunk& chunk, int32_t chunkX, int32_t chunkY);

    std::map<Vector2D, MapChunk> cacheMap;

    siv::PerlinNoise heightNoise;
    siv::PerlinNoise humidNoise;
    siv::PerlinNoise tempNoise;
    siv::PerlinNoise extraNoise;

};
