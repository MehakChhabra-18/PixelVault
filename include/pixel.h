#ifndef PIXEL_H
#define PIXEL_H

#include <stdint.h>

typedef struct
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;

int analyze_pixels(const char *filename);

uint8_t get_lsb(uint8_t value);

uint8_t set_lsb(uint8_t value, uint8_t bit);

#endif