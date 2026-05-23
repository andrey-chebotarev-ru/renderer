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
};

