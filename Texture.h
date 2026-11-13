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

class Texture
{
public:
    Texture() {}

    Texture(size_t width, size_t height, Color* pixels)
        : width(width), height(height), pixels(pixels)
    {
        //assert(Math::isPow2(width));
        shiftWidth = Math::shift(width);
    }

    size_t getWidth() const {
        return width;
    }

    size_t getHeight() const {
        return height;
    }

    inline const Color& getSample(float u, float v) const
    {
        /*
        int x = int(u) & (width - 1);
        int y = int(v) & (height - 1);

        int offset = x + (y << shiftWidth);
        */

        const int w = static_cast<int>(width);
        const int h = static_cast<int>(height);

        int x = static_cast<int>(std::floor(u)) % w;
        int y = static_cast<int>(std::floor(v)) % h;

        if (y < 0) {
            y += h;
        }

        if (x < 0) {
            x += w;
        }

        int offset = x + (y * width);


        /*
        int x = static_cast<int>(floorf(u));
        int y = static_cast<int>(floorf(v));

        x = ((x % width) + width) % width;
        y = ((y % height) + height) % height;
        */
        return pixels[y * width + x];
    }

    size_t width;
    size_t height;

    size_t shiftWidth;

    Color* pixels;

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
        size_t cellsSize = 8;

        const Color color1 = Color(255, 0, 255);
        const Color color2 = Color(0, 0, 0);

        Color* pointer = static_cast<Color*>(std::malloc(width * height * sizeof(Color)));

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                bool even = ((((x / cellsSize) + (y / cellsSize)) & 1) == 0);
                pointer[x + y * width] = even ? color1 : color2;
            }
        }

        pointers.push_back(pointer);
        textures.push_back(Texture(width, height, pointer));
    }

    ~TextureManager()
    {
        for (auto ptr : pointers) {
            std::free(ptr);
        }
    }

    uint32_t add(size_t width, size_t height, Color* pixels)
    {
        size_t texIdx = textures.size();
        Color* pointer = static_cast<Color*>(std::malloc(width * height * sizeof(Color)));

        std::memcpy(pointer, pixels, width * height * sizeof(Color));
        pointers.push_back(pointer);

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