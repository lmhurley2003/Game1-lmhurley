#include "asset_loader.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <fstream>
#include <array>

void printLoadData(std::vector<glm::u8vec4> data, uint32_t limit = 1000){
    for(uint32_t i = 0; i < data.size() && i < limit; i++) {
        /*std::cout << "(" << data[i].x << ", " << data[i].y << ", " <<  data[i].z << ", " <<  data[i].w << ")";*/
        std::cout << glm::to_string(data[i]);
        if(i < data.size() - 1) {
            std::cout <<"     ,     ";
        }
        std::cout << std::endl;
    }
}

bool loadAssets() {
    static_assert(sizeof(glm::u8vec4) == 4 && "u8vec4 should be 4 bytes in size");
    std::vector<glm::u8vec4> spriteData;
    glm::uvec2 atlasSize;
    load_png(data_path("assets/SpriteAtlas.png"), &atlasSize, &spriteData, UpperLeftOrigin);
    //printLoadData(spriteData);
    //set up indices of sprite atlas

    //lightning textures
    std::vector<InitialSpriteData> spriteInits;
    spriteInits.reserve(12);
    //uint32_t numTiles = 0;
    for(uint8_t y = 0; y < 3; y++){
        for(uint8_t x = 0; x < 4; x++) {
            spriteInits.emplace_back(InitialSpriteData(x, y));
        }
    }

    //ground tiles
    spriteInits.emplace_back(InitialSpriteData(4, 0));
    spriteInits.emplace_back(InitialSpriteData(5, 0));

    //playerTiles
    spriteInits.emplace_back(InitialSpriteData(4, 3));
    spriteInits.emplace_back(InitialSpriteData(4, 2));
    spriteInits.emplace_back(InitialSpriteData(4, 1));

    
    //load to binary file
    std::ofstream atlasBin(data_path("assets/atlas.bin"), std::ios::binary);

    for(uint32_t i = 0; i < spriteInits.size(); i++) {
        InitialSpriteData sprite = spriteInits[i];
        for(uint32_t y = sprite.y; y < static_cast<uint32_t>(sprite.y + sprite.h); y++) {
            for(uint32_t x = sprite.x; x < static_cast<uint32_t>(sprite.x + sprite.w); x++){
                assert(0 <= 8*x && 8*x < atlasSize.x);
                assert(0 <= 8*y && 8*y < atlasSize.y);
                
                //according to Tile datatype in PPU466.hpp
                std::array<uint8_t, 8> bit0s = {0, 0, 0, 0, 0, 0, 0, 0};
                std::array<uint8_t, 8> bit1s = {0, 0, 0, 0, 0, 0, 0, 0};
                for(uint32_t r = 0; r < 8; r++) {
                    for(int32_t c = 0; c < 8; c++) {
                        uint32_t idx = (y*8 + r)*atlasSize.x + x*8 + c;
                        assert(0 <= idx && idx < spriteData.size());
                        glm::u8vec4 col = spriteData[idx];
                        uint8_t bit0 = 0;
                        uint8_t bit1 = 0;
                        if(col.w < 100) { //if alpha 0...
                            //keep 00
                        } else {
                            // b/darkest = 01, g/midtone = 10, r/highest = 11
                            bit0 = static_cast<uint8_t>((col.x > 100u) || (col.z > 100u));
                            bit1 = static_cast<uint8_t>((col.x > 100u) || (col.y > 100u));
                        }
                        bit0s[7-r] = bit0s[7-r] | (bit0 << c);
                        bit1s[7-r] = bit1s[7-r] | (bit1 << c);
                    }
                }
                atlasBin.write(reinterpret_cast<char const *>(bit0s.data()), 8);
                atlasBin.write(reinterpret_cast<char const *>(bit1s.data()), 8);
            }
        }
    };

    //fill bin file to 256 capacity iwth empty tiles
    std::array<uint8_t, 8> emptyBits = {0, 0, 0, 0, 0, 0, 0, 0};
    for(uint32_t i = 0; i < 256 - spriteInits.size(); i++) {
        atlasBin.write(reinterpret_cast<char const *>(emptyBits.data()), 8);
        atlasBin.write(reinterpret_cast<char const *>(emptyBits.data()), 8);
    }
    

    return true;
}