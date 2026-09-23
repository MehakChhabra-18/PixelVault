#include <stdio.h>
#include "bmp.h"
#include "pixel.h"

int analyze_pixels(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error: Could not open %s\n", filename);
        return 1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, sizeof(BMPFileHeader), 1, file);
    fread(&info_header, sizeof(BMPInfoHeader), 1, file);

    if (file_header.signature != 0x4D42)
    {
        printf("Error: Not a valid BMP file.\n");
        fclose(file);
        return 1;
    }

    if (info_header.bits_per_pixel != 24)
    {
        printf("Error: Only 24-bit BMP images are supported.\n");
        fclose(file);
        return 1;
    }

    if (info_header.compression != 0)
    {
        printf("Error: Compressed BMP images are not supported.\n");
        fclose(file);
        return 1;
    }

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - IMAGE ANALYZER\n");
    printf("========================================\n\n");

    printf("Image          : %s\n", filename);
    printf("Width          : %d pixels\n", info_header.width);
    printf("Height         : %d pixels\n", info_header.height);
    printf("Bits Per Pixel : %u\n", info_header.bits_per_pixel);

    fseek(file, file_header.pixel_data_offset, SEEK_SET);

    printf("\nFirst 5 Pixels\n");
    printf("--------------\n");

    for (int i = 0; i < 5; i++)
    {
        Pixel pixel;

        fread(&pixel.blue, sizeof(uint8_t), 1, file);
        fread(&pixel.green, sizeof(uint8_t), 1, file);
        fread(&pixel.red, sizeof(uint8_t), 1, file);

        printf(
            "Pixel %d : B=%u G=%u R=%u\n",
            i + 1,
            pixel.blue,
            pixel.green,
            pixel.red
        );
    }

    printf("\n========================================\n");

    fclose(file);

    return 0;
}

uint8_t get_lsb(uint8_t value)
{
    return value & 1;
}

uint8_t set_lsb(uint8_t value, uint8_t bit)
{
    value = value & ~1;
    value = value | bit;

    return value;
}