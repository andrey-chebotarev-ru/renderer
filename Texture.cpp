#include "Texture.h"

#include "Math.h"
#include "Color.h"

Texture::Texture()
{
    //assert(Math::isPow2(width) && Math::isPow2(height));

   // shiftWidth = Math::shift(width);

    /*
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int offset = x + (y << shiftWidth);
            pixels[offset] = Color(255, 0, 0);
        }
    }
    */
}

Texture::Texture(size_t width, size_t height, Color* pixels)
    : width(width), height(height), pixels(pixels)
{
    assert(Math::isPow2(width));

    shiftWidth = Math::shift(width);
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
    int y = int(v) % height;

    int offset = x + (y << shiftWidth);

    return pixels[offset];
}