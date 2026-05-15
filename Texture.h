#pragma once

#include <cstdint>
#include <assert.h>

#include "Math.h"
#include "Color.h"

class Texture
{
public:
    Texture() {}
    Texture(size_t width, size_t height, int pitch, uint8_t* pixels);

    size_t getWidth() const;
    size_t getHeight() const;
    Color getSample(float u, float v) const;
private:
    size_t width;
    size_t height;
    int pitch;
    int shiftWidth;
    int shiftPitch;

    uint8_t* pixels;
};