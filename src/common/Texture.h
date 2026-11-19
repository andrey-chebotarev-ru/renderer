#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <array>

#include "Common.h"
#include "Math.h"
#include "Color.h"
#include "Debug.h"
//#include "Vector.h"
//#include "Vertex.h"

//#include "Triangle.h"


class Texture
{
public:
    Texture() {}

    Texture(size_t width, size_t height, Color* pixels[4])
        : width(width), height(height)
    {
        this->pixels[0] = pixels[0];
        this->pixels[1] = pixels[1];
        this->pixels[2] = pixels[2];
        this->pixels[3] = pixels[3];

        scale[0] = 1.0f;
        scale[1] = 1.0f / float(1 << 1);
        scale[2] = 1.0f / float(1 << 2);
        scale[3] = 1.0f / float(1 << 3);

        //assert(Math::isPow2(width));
        shiftWidth = Math::shift(width);
    }

    size_t getWidth() const {
        return width;
    }

    size_t getHeight() const {
        return height;
    }

    inline const Color& getSample(float u, float v, uint8_t mipLevel) const
    {
        /*
        int x = int(u) & (width - 1);
        int y = int(v) & (height - 1);

        int offset = x + (y << shiftWidth);
        */

        const int w = static_cast<int>(width >> mipLevel);
        const int h = static_cast<int>(height >> mipLevel);

        int x = static_cast<int>(std::floor(u * scale[mipLevel])) % w;
        int y = static_cast<int>(std::floor(v * scale[mipLevel])) % h;

        if (y < 0) {
            y += h;
        }

        if (x < 0) {
            x += w;
        }

        int offset = x + (y * w);


        /*
        int x = static_cast<int>(floorf(u));
        int y = static_cast<int>(floorf(v));

        x = ((x % width) + width) % width;
        y = ((y % height) + height) % height;
        */
        return pixels[mipLevel][y * w + x];
    }

    size_t width;
    size_t height;

    size_t shiftWidth;

    Color* pixels[4];
    float scale[4];

};

using TextureList = std::vector<Texture>;

class TextureManager
{
public:

    const size_t invalidTexIdx = 0;

    TextureManager()
    {
        size_t width = 32;
        size_t height = 32;
        size_t cellsSize = 16;

        const Color color1 = Color(255, 0, 255);
        const Color color2 = Color(0, 0, 0);

        Color* pointer[4];

        for (int mipLevel = 0; mipLevel < 4; mipLevel++) {
            int w = width >> mipLevel;
            int h = height >> mipLevel;
            int c = cellsSize >> mipLevel;
            pointer[mipLevel] = static_cast<Color*>(std::malloc(w * h * sizeof(Color)));

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    bool even = ((((x / c) + (y / c)) & 1) == 0);
                    pointer[mipLevel][x + y * w] = even ? color1 : color2;
                }
            }

            pointers.push_back(pointer[mipLevel]);
        }
        
        textures.push_back(Texture(width, height, pointer));
    }

    ~TextureManager()
    {
        for (auto ptr : pointers) {
            std::free(ptr);
        }
    }

    uint32_t add(size_t width, size_t height, Color* pixels[4])
    {
        size_t texIdx = textures.size();
        
        Color* pointer[4];

        for (int mipLevel = 0; mipLevel < 4; mipLevel++) {
            int w = width >> mipLevel;
            int h = height >> mipLevel;
            pointer[mipLevel] = static_cast<Color*>(std::malloc(w * h * sizeof(Color)));

            std::memcpy(pointer[mipLevel], pixels[mipLevel], w * h * sizeof(Color));
            pointers.push_back(pointer[mipLevel]);
        }

        textures.push_back(Texture(width, height, pointer));
        return texIdx;
    }

    const Texture& get(size_t texIdx) const
    {
        return textures[texIdx];
    }

    const size_t getTextureCount() const
    {
        return textures.size();
    }

private:

    Texture defaultTex;

    TextureList textures;
    std::vector<Color*> pointers;
};

class TextureTable
{
public:

    uint32_t add(std::string name)
    {
       // std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        if (auto it = table.find(name); it != table.end()) {
            return it->second;
        }

        uint32_t idx = static_cast<uint32_t>(names.size());
        
        names.push_back(name);
        table.emplace(name, idx);
        
        return idx;
    }

    const StringList& getNames() const
    {
        return names;
    }

private:

    std::unordered_map<std::string, uint32_t> table;
    StringList names;
};



