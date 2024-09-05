#include "asset_loader.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"
#include <glm/glm.hpp>

#include <vector>
#include <iostream>

void printLoadData(std::vector<glm::u8vec4> data, uint32_t limit = 1000){
    for(uint32_t i = 0; i < data.size() && i < limit; i++) {
        std::cout << "(" << data[i].x << ", " << data[i].y << ", " <<  data[i].z << ", " <<  data[i].w << ")";
        if(i < data.size() - 1) {
            std::cout <<"     ,     ";
        }
    }
}

bool loadAssets() {

    std::vector<glm::u8vec4> spriteData;
    glm::uvec2 atlasSize;
    load_png(data_path("assets/SpriteAtlas.png"), &atlasSize, &spriteData, UpperLeftOrigin);

    printLoadData(spriteData);
    return true;
}