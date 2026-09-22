#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct 
{
    uint16_t signature;  //unsigned integer occupying 16 bits 
    uint32_t file_size;  //unsigned integer occupying 32 bits
    uint32_t pixel_data_offset;
} BMPFileHeader; 


typedef struct 
{
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t image_size;
} BMPInfoHeader;

int read_bmp_info(const char *filename);
#endif


