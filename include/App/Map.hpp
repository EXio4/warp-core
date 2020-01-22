#pragma once

#include <cstdint>
#include <utility> // for std::pair
#include <map>
#include "Ext/PerlinNoise.hpp"


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

    TileData() {};
    TileData(TileBiome biome, TileBehavior behavior, TileType type, uint32_t color, uint32_t height, double light)
        : biome(biome), behavior(behavior), type(type), color(color), height(height), light(light) {
        }
};

using Vector2D = std::pair<int32_t, int32_t>;

struct MapChunk {
    std::vector<TileData> data;
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

    TileData genTile(double x, double y, bool calculateLight);
    MapChunk mapgen(int32_t chunkX, int32_t chunkY);

    std::map<Vector2D, MapChunk> cacheMap;

    siv::PerlinNoise heightNoise;
    siv::PerlinNoise humidNoise;
    siv::PerlinNoise tempNoise;
    siv::PerlinNoise extraNoise;
};
