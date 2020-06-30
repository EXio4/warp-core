#include <cstdint>
#include <stdio.h>
#include <cmath>

#include "App/Map.hpp"
#include "App/utils.hpp"

std::map<TileType, uint32_t> colorMap = {
    { Grass, rgba(0x98, 0xdd, 0x00, 0xff) },
    { Water, rgba(0x00, 0xdd, 0xca, 0xff) },
    { Sand , rgba(0xc2, 0xb2, 0x80, 0xff) },
    { DesertSand, rgba(0xf4, 0xa4, 0x60, 0xff) },
    { Dirt, rgba(0xbb, 0x8b, 0x00, 0xff) },
    { Stone, rgba(0x8d, 0x8d, 0x8d, 0xff) },
    { Ice, rgba(0xb9, 0xe8, 0xea, 0xff) },
    { Snow, rgba(0xff, 0xfa, 0xfa, 0xff) },
    { Lava, rgba(0xcf, 0x10, 0x20, 0xff) },
    { VolcanicRock, rgba(0x3d, 0x3f, 0x3e, 0xff) },
};

uint32_t getColor(TileType type) {
    return colorMap[type];
}

Map::Map() {

    heightNoise.SetNoiseType(FastNoise::Perlin);
    tempNoise.SetNoiseType(FastNoise::Perlin);
    biomeNoise.SetNoiseType(FastNoise::Perlin);
    extraNoise.SetNoiseType(FastNoise::Perlin);

    baselineBrightness = 20;
    sunlightBrightness = 100;
    sunlightAngle = M_PI/3;
}

Map::Map(uint32_t seed) {
    heightNoise.SetSeed(seed);
    tempNoise.SetSeed(3*seed + 2);
    biomeNoise.SetSeed(5*seed + 3);
    extraNoise.SetSeed(7*seed + 4);

    heightNoise.SetNoiseType(FastNoise::ValueFractal);
    heightNoise.SetFrequency(1./HEIGHT_RATIO);
    heightNoise.SetFractalOctaves(8);
    tempNoise.SetNoiseType(FastNoise::SimplexFractal);
    tempNoise.SetFractalOctaves(5);
    tempNoise.SetFrequency(1./TEMP_RATIO);
    biomeNoise.SetNoiseType(FastNoise::SimplexFractal);
    biomeNoise.SetFractalOctaves(4);
    biomeNoise.SetFrequency(1./BIOME_RATIO);
    extraNoise.SetNoiseType(FastNoise::PerlinFractal);

    baselineBrightness = 20;
    sunlightBrightness = 100;
    sunlightAngle = M_PI/3;
}

void Map::genTile(TileData& res, double x, double y, bool calculateLight) {
    TileBiome biome = BFields;
    TileBehavior behavior = TSolid;
    TileType type = Water;


    double baseHeightN = 80 * (heightNoise.GetNoise(x, y) + 1);
    double biomeN = 50 * (biomeNoise.GetNoise(x, y) + 1.2);
    double internalBiomeN  = 32.5 * (tempNoise.GetNoise(x, y) + 1) - 15;

    double ratio = 100;
    {
        double x = biomeN;
        if (x >= 0 && x < 2) {
            ratio = -0.9587529719497954*x*x*x-4.13559879838381e-59*x*x+3.8350118877991815*x+100;
        } else if (x < 5) {
            ratio = 0.9178789560031126*x*x*x-11.259791567717448*x*x+26.35459502323408*x+84.98694457637674;
        } else if (x < 20) {
            ratio = -0.07803033762028887*x*x*x+3.678847836633574*x*x-48.338601998521035*x+209.47560627930193;
        } else if (x < 24) {
            ratio = 0.08331145999123345*x*x*x-6.0016600200577646*x*x+145.27155513530573*x-1081.2587746128766;
        } else if (x < 50) {
            ratio = -0.0010138376255269096*x*x*x+0.06976140834898119*x*x-0.4425591464561576*x+84.45413964121862;
        } else if (x < 80) {
            ratio = +0.003562549148975762*x*x*x-0.6166966078264196*x*x+33.88034166231388*x-487.5942071716154;
        } else if (x < 90) {
            ratio = -0.009929799496990138*x*x*x+2.621467067205397*x*x-225.17275234023143*x+6420.488299562926;
        } else {
            ratio = 0.0019859598993980277*x*x*x-0.5957879698194083*x*x+64.38020099200104*x-2266.1003004040476;
        }
    }
    double height = baseHeightN; // * (ratio / 100);

    // the biome affects the baseHeight as a modifier, by a different margin depending on where it is based on the spline interpolation by:
    /*
        0 100
        2 100
        5 50
        20 90
        24 100
        50 110
        80 100
        90 150
        100 200
    */
    // biome 
    // [0;5) island
    // [5;20) ocean
    // [20;24) beach
    // [24;90) "rest"
    // [90;100] mountain
    // internalBiome defines what we see on the main biome, and it's basically a temperature ratio mixed in with other stuff
    // [-15; 0) frozen/icy (might be a frozen lake, snow island, or such)
    // [0;2) transition phase
    // [2; 26) normal, defines grass fields, and forests
    // [26; 33) transition phase
    // [33; 37) deserts
    // [37; 40) transition phase
    // [40; 50] hell 

    // sea level is defined at lvl 30

    if (internalBiomeN <= 0) {
        biome = BIce;
        if (height <= 30) {
            type = Ice;
        } else {
            type = Snow;
        }
    } else if (internalBiomeN < 2) {
        biome = BIce;
        if (height <= 30) {
            type = Water;
        } else {
            type = Snow;
        }
    } else if (internalBiomeN < 26) {
        biome = BFields;
        if (height <= 30) {
            type = Water;
        } else if (height <= 40) {
            type = Sand;
        } else if (height <= 170) {
            type = Grass;
        } else if (height <= 200) {
            type = Dirt;
        } else {
            type = Stone;
        }
    } else if (internalBiomeN < 33) {
        biome = BDesert;
        if (height <= 30) {
            type = Water;
        } else if (height <= 50) {
            type = Sand;
        } else if (height <= 150) {
            type = DesertSand;
        } else if (height <= 200) {
            type = Stone;
        } else {
            type = VolcanicRock;
        }
    } else if (internalBiomeN < 37) {
        biome = BDesert;
        if (height <= 30) {
            type = Water;
        } else if (height <= 50) {
            type = Sand;
        } else if (height <= 170) {
            type = DesertSand;
        } else if (height <= 200) {
            type = Stone;
        } else {
            type = VolcanicRock;
        }
    } else {
        biome = BHell;
        if (height <= 30) {
            type = Lava;
        } else if (height <= 50) {
            type = VolcanicRock;
        } else if (height <= 170) {
            type = Stone;
        } else if (height <= 200) {
            type = VolcanicRock;
        } else {
            type = Stone;
        }
    }
    // override biome for correct definition
    if (biomeN <= 4) {
        biome = BIsland;
    } else if (biomeN <= 20) {
        biome = BOcean;
    } else if (biomeN >= 90) {
        biome = BMountains;
    }

    if (type == Water) {
        behavior = TReflectiveLiquid;
    }
    if (type == Lava) {
        behavior = TLiquid;
    }

    height *= 1.25;

    // range for values:
    // baseHeight = 0-200
    // biome = 0 to 100
    // temp = -15 to 50


    // if (temp <= 0) {
    //     biome = BIce;
    //     // icy 
    //     if (noise <= 6) {
    //         type = Ice;
    //         noise = 5;
    //     } else {
    //         type = Snow;
    //     }
    // } else if (temp <= 40) {
    //     biome = BNormal;
    //     // normal
    //     if (humid <= 25 && noise <= 7) {
    //         // adjust water level due to humidity
    //         noise += 5 * (25-humid)/25;
    //     }
    //     if (noise <= 6) {
    //         if (temp <= 2.5) {
    //             biome = BIce;
    //             type = Ice;
    //         } else if (temp <= 38) {
    //             type = Water;
    //             behavior = TReflectiveLiquid;
    //         } else {
    //             biome = BHell;
    //             type = Lava;
    //             behavior = TLiquid;
    //         }
    //         noise = 5;
    //     } else if (humid <= 25) {
    //         biome = BDesert;
    //         type = DesertSand;
    //     } else if (noise <= 15) {
    //         biome = BDesert;
    //         type = Sand;
    //     } else if (noise <= 25) {
    //         type = Grass;
    //     } else {
    //         type = Stone;
    //     }
    // } else {
    //     biome = BHell;
    //     // hell-ish
    //     if (noise <= 6) {
    //         type = Lava;
    //         noise = 5;
    //         behavior = TLiquid;
    //     } else {
    //         type = VolcanicRock;
    //     }
    // }

    double light = sunlightBrightness;
    uint32_t color = getColor(type);

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
void Map::mapgen(MapChunk& chunk, int32_t chunkX, int32_t chunkY) {

    for (int32_t dy = 0; dy < CHUNK_SIZE; dy++) {
        for (int32_t dx = 0; dx < CHUNK_SIZE; dx++) {
            double x = chunkX * CHUNK_SIZE + dx;
            double y = chunkY * CHUNK_SIZE + dy;
            genTile(chunk.data[dx * CHUNK_SIZE + dy], x, y, true);
        }
    }
    chunk.initialized = true;
}

TileData Map::get(double _x, double _y) {
    const div_t& pX = div_floor(_x, CHUNK_SIZE);
    const div_t& pY = div_floor(_y, CHUNK_SIZE);

    int32_t chunkX = pX.quot;
    int32_t chunkY = pY.quot;
    uint32_t offsetX = pX.rem;
    uint32_t offsetY = pY.rem;

    
    const auto& chunkIter = cacheMap.find({ chunkX, chunkY });
    if (chunkIter != cacheMap.end()) {
        return chunkIter->second.data[offsetX * CHUNK_SIZE + offsetY];
    } else {
        Vector2D pos = std::make_pair(chunkX, chunkY);
        MapChunk chunk;
        mapgen(chunk, chunkX, chunkY);
        cacheMap.insert({ pos, chunk });
        return chunk.data[offsetX * CHUNK_SIZE + offsetY];
    }
}