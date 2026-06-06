#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <assert.h>

#include "Math.h"
#include "Color.h"

class Texture
{
public:
    Texture();
    Texture(size_t width, size_t height, size_t pitch, Color* pixels);

    size_t getWidth() const;
    size_t getHeight() const;
    Color getSample(float u, float v) const;

private:
    size_t width;
    size_t height;
    
    size_t shiftWidth;

    Color* pixels;
};

class TextureManager
{
public:

    const size_t invalidTexIdx = 0xffffffff;

    TextureManager()
    {
       
    }

    ~TextureManager()
    {
        for (auto ptr : dataList) {
            std::free(ptr);
        }
    }

    bool add(const std::string& name, size_t width, size_t height, size_t pitch, uint8_t* data)
    {
        if (names.contains(name)) {
            return false;
        }
        
        size_t texIdx = textures.size();

        names[name] = texIdx;

        Color* dstData = static_cast<Color*>(std::malloc(width * height * sizeof(Color)));


        for (int y = 0; y < height; y++) {
            Color* srcRow = reinterpret_cast<Color*>(data + y * pitch);
            Color* dstRow = dstData + y * width;

            std::memcpy(dstRow, srcRow, width * sizeof(Color));
        }

        dataList.push_back(dstData);

        textures.push_back(Texture(width, height, pitch, dstData));
        return true;
    }

    Texture get(const std::string& name) const
    {
        auto iter = names.find(name);

        if (iter == names.end()) {
            return defaultTex;
        }

        return textures[iter->second];
    }

    Texture get(size_t texIdx) const
    {
        return textures[texIdx];
    }

    size_t getIdx(const std::string& name) const
    {
        auto iter = names.find(name);

        if (iter == names.end()) {
            return invalidTexIdx;
        }

        return iter->second;
    }

private:

    Texture defaultTex;

    std::unordered_map<std::string, size_t> names;
    std::vector<Texture> textures;
    std::vector<Color*> dataList;
};