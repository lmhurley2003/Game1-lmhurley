#include <iostream>

#include <glm/glm.hpp>
#include <vector>

struct InitialSpriteData{
    //in 8-pixel chunks, from upper left of sprite
    uint8_t x; 
    uint8_t y;
    uint8_t w = 1;
    uint8_t h = 1;

    InitialSpriteData(uint8_t _x, uint8_t _y) : x(_x), y(_y) {}
};

void printLoadData(std::vector<glm::u8vec4> data, uint32_t limit);

bool loadAssets();