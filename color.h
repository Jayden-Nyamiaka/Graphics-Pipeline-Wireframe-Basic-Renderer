#ifndef COLOR_H
#define COLOR_H

#include <cstddef>
#include <string>

typedef struct color_rgb {
    std::size_t r;
    std::size_t g;
    std::size_t b;
} color_rgb_t;

color_rgb_t initColor(std::size_t red, std::size_t green, std::size_t blue);

color_rgb_t scaleColor(color_rgb_t color, float factor);

color_rgb_t randColor();

color_rgb_t negateColor(color_rgb_t color);

std::string toString(color_rgb_t color);

#endif