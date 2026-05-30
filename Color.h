#pragma once

#include <cstdint>

struct Color
{
    union {
        struct {
            uint8_t r, g, b, a;
        };
        uint32_t val;
    };

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    Color();

    static Color getDebugColor(int idx)
    {
        Color color;

        color.r = (idx * 97) % 256;
        color.g = (idx * 57) % 256;
        color.b = (idx * 23) % 256;
        color.a = 255;

        return color;
    }
};

