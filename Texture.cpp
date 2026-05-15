#include "Texture.h"

#include "Math.h"
#include "Color.h"

Texture::Texture(size_t width, size_t height, int pitch, uint8_t* pixels)
    : width(width), height(height), pitch(pitch), pixels(pixels)
{
    assert(Math::isPow2(width) && Math::isPow2(pitch));

    shiftWidth = Math::shift(width);
    shiftPitch = Math::shift(pitch);
}

size_t Texture::getWidth() const
{
    return width;
}

size_t Texture::getHeight() const
{
    return height;
}

Color Texture::getSample(float u, float v) const
{
    int x = int(u) & (width - 1);
    int y = int(v) & (height - 1);

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return Color();
    }

    int index = (x << 2) + (y << shiftPitch);

    return *(Color*)(pixels + index);
}