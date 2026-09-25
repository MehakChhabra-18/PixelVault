#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bmp.h"
#include "encoder.h"


/*
 * Calculate how many bytes of secret data
 * can be stored in the image.
 *
 * We use 1 bit from every actual pixel byte.
 *
 * 8 pixel bytes = 1 message byte.
 */
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

    /*
     * Every pixel has:
     *
     * Blue  = 1 byte
     * Green = 1 byte
     * Red   = 1 byte
     *
     * Therefore:
     *
     * 3 bytes per pixel.
     */
    int pixel_bytes = width * height * 3;

    /*
     * 8 pixel bytes are required
     * to store 1 message byte.
     */
    int capacity = pixel_bytes / 8;

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - IMAGE CAPACITY\n");
    printf("========================================\n\n");

    printf("Image            : %s\n", filename);
    printf("Width            : %d pixels\n", width);
    printf("Height           : %d pixels\n", height);
    printf("Pixel Bytes      : %d\n", pixel_bytes);
    printf("Message Capacity : %d bytes\n", capacity);

    printf("\n========================================\n");

    fclose(file);

    return capacity;
}


/*
 * Hide a message inside a BMP image.
 *
 * Format stored inside image:
 *
 * [32-bit message length]
 * [message bytes]
 */
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

    /*
     * Validate BMP.
     */
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
        printf(
            "Error: Only uncompressed 24-bit BMP files are supported.\n"
        );

        fclose(input);
        fclose(output);

        return 1;
    }


    /*
     * Calculate message length.
     */
    int message_length = 0;

    while (message[message_length] != '\0')
    {
        message_length++;
    }


    /*
     * Calculate capacity.
     */
    int capacity = calculate_capacity(input_filename);

    if (capacity < 0)
    {
        fclose(input);
        fclose(output);

        return 1;
    }


    /*
     * We need 4 extra bytes to store
     * the message length.
     */
    if (message_length + 4 > capacity)
    {
        printf("\n");
        printf("Error: Message is too large for this image.\n");
        printf("Required : %d bytes\n", message_length + 4);
        printf("Available: %d bytes\n", capacity);

        fclose(input);
        fclose(output);

        return 1;
    }


    /*
     * Copy everything before pixel data
     * exactly as it is.
     */
    fseek(input, 0, SEEK_SET);

    unsigned char byte;

    for (uint32_t i = 0;
         i < file_header.pixel_data_offset;
         i++)
    {
        fread(&byte, 1, 1, input);
        fwrite(&byte, 1, 1, output);
    }


    /*
     * Move to actual pixel data.
     */
    fseek(input, file_header.pixel_data_offset, SEEK_SET);

    /*
     * Skip to output pixel position.
     *
     * The output file is already positioned
     * after the copied header.
     */


    /*
     * ------------------------------------------------
     * STORE MESSAGE LENGTH
     * ------------------------------------------------
     *
     * Message length is stored using 32 bits.
     */
    uint32_t length = (uint32_t)message_length;

    for (int bit_index = 31;
         bit_index >= 0;
         bit_index--)
    {
        fread(&byte, 1, 1, input);

        unsigned char bit =
            (length >> bit_index) & 1;

        /*
         * Clear the LSB.
         */
        byte = byte & 0xFE;

        /*
         * Put our secret bit into LSB.
         */
        byte = byte | bit;

        fwrite(&byte, 1, 1, output);
    }


    /*
     * ------------------------------------------------
     * STORE MESSAGE
     * ------------------------------------------------
     */
    for (int char_index = 0;
         char_index < message_length;
         char_index++)
    {
        unsigned char current_char =
            (unsigned char)message[char_index];

        /*
         * Store 8 bits of the character.
         */
        for (int bit_index = 7;
             bit_index >= 0;
             bit_index--)
        {
            fread(&byte, 1, 1, input);

            unsigned char bit =
                (current_char >> bit_index) & 1;

            /*
             * Clear existing LSB.
             */
            byte = byte & 0xFE;

            /*
             * Insert secret bit.
             */
            byte = byte | bit;

            fwrite(&byte, 1, 1, output);
        }
    }


    /*
     * ------------------------------------------------
     * COPY REMAINING IMAGE DATA
     * ------------------------------------------------
     *
     * Everything after the encoded data remains
     * unchanged.
     */
    while (fread(&byte, 1, 1, input) == 1)
    {
        fwrite(&byte, 1, 1, output);
    }


    fclose(input);
    fclose(output);


    /*
     * Success message.
     */
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


/*
 * Extract a hidden message from a BMP image.
 */
int extract_message(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error: Could not open image.\n");
        return 1;
    }


    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, sizeof(BMPFileHeader), 1, file);
    fread(&info_header, sizeof(BMPInfoHeader), 1, file);


    /*
     * Validate BMP.
     */
    if (file_header.signature != 0x4D42)
    {
        printf("Error: Invalid BMP file.\n");

        fclose(file);

        return 1;
    }

    if (info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf(
            "Error: Only uncompressed 24-bit BMP files are supported.\n"
        );

        fclose(file);

        return 1;
    }


    /*
     * Move to pixel data.
     */
    fseek(
        file,
        file_header.pixel_data_offset,
        SEEK_SET
    );


    /*
     * ------------------------------------------------
     * READ MESSAGE LENGTH
     * ------------------------------------------------
     *
     * First 32 pixel LSBs contain
     * the message length.
     */
    uint32_t message_length = 0;

    for (int bit_index = 31;
         bit_index >= 0;
         bit_index--)
    {
        unsigned char byte;

        if (fread(&byte, 1, 1, file) != 1)
        {
            printf("Error: Could not read hidden data.\n");

            fclose(file);

            return 1;
        }

        unsigned char bit = byte & 1;

        message_length =
            message_length |
            ((uint32_t)bit << bit_index);
    }


    /*
     * Basic validation.
     */
    if (message_length == 0)
    {
        printf("No hidden message found.\n");

        fclose(file);

        return 1;
    }


    /*
     * Prevent unreasonable memory allocation.
     */
    if (message_length > 1000000)
    {
        printf("Error: Invalid hidden message.\n");

        fclose(file);

        return 1;
    }


    /*
     * Make sure message can actually fit
     * inside the image.
     */
    int capacity = calculate_capacity(filename);

    if (capacity < 0 ||
        message_length + 4 > (uint32_t)capacity)
    {
        printf("Error: Invalid or corrupted hidden message.\n");

        fclose(file);

        return 1;
    }


    /*
     * Allocate memory for message.
     *
     * +1 is for '\0'.
     */
    char *message =
        (char *)malloc(message_length + 1);

    if (message == NULL)
    {
        printf("Error: Memory allocation failed.\n");

        fclose(file);

        return 1;
    }


    /*
     * ------------------------------------------------
     * READ MESSAGE
     * ------------------------------------------------
     */
    for (uint32_t char_index = 0;
         char_index < message_length;
         char_index++)
    {
        unsigned char value = 0;

        /*
         * Read 8 pixel LSBs.
         */
        for (int bit_index = 7;
             bit_index >= 0;
             bit_index--)
        {
            unsigned char byte;

            if (fread(&byte, 1, 1, file) != 1)
            {
                printf("Error: Could not read hidden message.\n");

                free(message);
                fclose(file);

                return 1;
            }

            unsigned char bit = byte & 1;

            value =
                value |
                (bit << bit_index);
        }

        message[char_index] = (char)value;
    }


    /*
     * Add string terminator.
     */
    message[message_length] = '\0';


    /*
     * Display result.
     */
    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - EXTRACT\n");
    printf("========================================\n\n");

    printf("Image          : %s\n", filename);
    printf("Message Length : %u bytes\n", message_length);

    printf("\nHidden Message:\n");
    printf("%s\n", message);

    printf("\n========================================\n");


    free(message);
    fclose(file);

    return 0;
}