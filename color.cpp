#include <stdlib.h>

#include "color.h"

color_rgb_t initColor(std::size_t red, std::size_t green, std::size_t blue) {
    color_rgb_t color{red, green, blue};
    return color;
}

color_rgb_t scaleColor(color_rgb_t color, float factor) {
    return initColor((int)(factor * color.r),
                     (int)(factor * color.g),
                     (int)(factor * color.b));
}

size_t randNum(size_t min, size_t max) {
    return (size_t)( (rand() * 1.0 * (max - min + 1)) / RAND_MAX + min);
}

color_rgb_t randColor() {
    size_t r = randNum(0, 255);
    size_t g = randNum(0, 255);
    size_t b = randNum(0, 255);
    if (r > 110 && r < 145 && g > 110 && g < 145) {
        b = randNum(1, 100);
        b += (randNum(0, 1) == 0) ? 0 : 155;
    }
    return initColor(r, g, b);
}

color_rgb_t negateColor(color_rgb_t color) {
    return initColor(255 - color.r, 255 - color.g, 255 - color.b);
}

std::string toString(color_rgb_t color) {
    return std::to_string(color.r) + " " + std::to_string(color.g) 
                                + " " + std::to_string(color.b);
}