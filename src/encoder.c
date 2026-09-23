#include <stdio.h>
#include "bmp.h"
#include "encoder.h"

int calculate_capacity(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error: Could not open %s\n", filename);
        return -1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, sizeof(BMPFileHeader), 1, file);
    fread(&info_header, sizeof(BMPInfoHeader), 1, file);

    if (file_header.signature != 0x4D42)
    {
        printf("Error: Not a valid BMP file.\n");
        fclose(file);
        return -1;
    }

    if (info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf("Error: Only uncompressed 24-bit BMP files are supported.\n");
        fclose(file);
        return -1;
    }

    int width = info_header.width;
    int height = info_header.height;

    int bytes_per_row = width * 3;

    int padding = (4 - (bytes_per_row % 4)) % 4;

    int total_pixel_bytes =
        (bytes_per_row + padding) * height;

    int capacity = total_pixel_bytes / 8;

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - IMAGE CAPACITY\n");
    printf("========================================\n\n");

    printf("Image            : %s\n", filename);
    printf("Width            : %d pixels\n", width);
    printf("Height           : %d pixels\n", height);
    printf("Pixel Bytes      : %d\n", total_pixel_bytes);
    printf("Message Capacity : %d bytes\n", capacity);

    printf("\n========================================\n");

    fclose(file);

    return capacity;
}

int hide_message(
    const char *input_filename,
    const char *output_filename,
    const char *message
)
{
    FILE *input = fopen(input_filename, "rb");

    if (input == NULL)
    {
        printf("Error: Could not open input image.\n");
        return 1;
    }

    FILE *output = fopen(output_filename, "wb");

    if (output == NULL)
    {
        printf("Error: Could not create output image.\n");
        fclose(input);
        return 1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, sizeof(BMPFileHeader), 1, input);
    fread(&info_header, sizeof(BMPInfoHeader), 1, input);

    if (file_header.signature != 0x4D42)
    {
        printf("Error: Invalid BMP file.\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    if (info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf("Error: Only uncompressed 24-bit BMP files are supported.\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    int message_length = 0;

    while (message[message_length] != '\0')
    {
        message_length++;
    }

    int capacity = calculate_capacity(input_filename);

    if (message_length > capacity)
    {
        printf("Error: Message is too large for this image.\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    /*
     * Copy BMP headers to output.
     */

    fseek(input, 0, SEEK_SET);

    unsigned char header_byte;

    for (int i = 0; i < file_header.pixel_data_offset; i++)
    {
        fread(&header_byte, 1, 1, input);
        fwrite(&header_byte, 1, 1, output);
    }

    /*
     * Encode message.
     */

    int message_index = 0;
    int bit_index = 0;

    unsigned char pixel_byte;

    for (int i = 0;
         i < message_length * 8;
         i++)
    {
        fread(&pixel_byte, 1, 1, input);

        unsigned char current_bit =
            (message[message_index] >> (7 - bit_index)) & 1;

        pixel_byte =
            (pixel_byte & 0xFE) | current_bit;

        fwrite(&pixel_byte, 1, 1, output);

        bit_index++;

        if (bit_index == 8)
        {
            bit_index = 0;
            message_index++;
        }
    }

    /*
     * Copy remaining image data unchanged.
     */

    while (fread(&pixel_byte, 1, 1, input) == 1)
    {
        fwrite(&pixel_byte, 1, 1, output);
    }

    fclose(input);
    fclose(output);

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - HIDE\n");
    printf("========================================\n\n");

    printf("Input Image  : %s\n", input_filename);
    printf("Output Image : %s\n", output_filename);
    printf("Message      : %s\n", message);
    printf("Message Size : %d bytes\n", message_length);

    printf("\nMessage successfully hidden!\n");
    printf("Output saved to: %s\n", output_filename);

    printf("\n========================================\n");

    return 0;
}