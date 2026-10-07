#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "bmp.h"
#include "encoder.h"
#include "payload.h"

/*
 * Calculate available payload capacity.
 *
 * 1 bit is stored in every image byte.
 *
 * First 32 bits are reserved for the old
 * text-message length format.
 */
int calculate_capacity(
    const char *filename)
{
    FILE *file =
        fopen(filename, "rb");

    if (file == NULL)
    {
        printf(
            "Error: Could not open %s\n",
            filename);

        return -1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    if (
        fread(
            &file_header,
            sizeof(BMPFileHeader),
            1,
            file) != 1)
    {
        fclose(file);

        return -1;
    }

    if (
        fread(
            &info_header,
            sizeof(BMPInfoHeader),
            1,
            file) != 1)
    {
        fclose(file);

        return -1;
    }

    if (file_header.signature != 0x4D42)
    {
        printf(
            "Error: Not a valid BMP file.\n");

        fclose(file);

        return -1;
    }

    if (
        info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf(
            "Error: Only uncompressed "
            "24-bit BMP files are supported.\n");

        fclose(file);

        return -1;
    }

    int width =
        info_header.width;

    int height =
        info_header.height;

    int bytes_per_row =
        width * 3;

    int padding =
        (4 - (bytes_per_row % 4)) % 4;

    int total_pixel_bytes =
        (bytes_per_row + padding) *
        height;

    /*
     * 4 bytes reserved for old
     * text-message length.
     */
    int capacity =
        (total_pixel_bytes / 8) - 4;

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - IMAGE CAPACITY\n");
    printf("========================================\n\n");

    printf(
        "Image            : %s\n",
        filename);

    printf(
        "Width            : %d pixels\n",
        width);

    printf(
        "Height           : %d pixels\n",
        height);

    printf(
        "Pixel Bytes      : %d\n",
        total_pixel_bytes);

    printf(
        "Message Capacity : %d bytes\n",
        capacity);

    printf(
        "\n========================================\n");

    fclose(file);

    return capacity;
}

/*
 * Old text-message hiding functionality.
 *
 * Kept so your existing project continues to work.
 */
int hide_message(
    const char *input_filename,
    const char *output_filename,
    const char *message)
{
    if (
        input_filename == NULL ||
        output_filename == NULL ||
        message == NULL)
    {
        printf(
            "Error: Invalid input parameters.\n");

        return 1;
    }

    if (
        strcmp(
            input_filename,
            output_filename) == 0)
    {
        printf(
            "Error: Input and output images "
            "must be different.\n");

        return 1;
    }

    FILE *input =
        fopen(input_filename, "rb");

    if (input == NULL)
    {
        printf(
            "Error: Could not open input image.\n");

        return 1;
    }

    FILE *output =
        fopen(output_filename, "wb");

    if (output == NULL)
    {
        printf(
            "Error: Could not create output image.\n");

        fclose(input);

        return 1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(
        &file_header,
        sizeof(BMPFileHeader),
        1,
        input);

    fread(
        &info_header,
        sizeof(BMPInfoHeader),
        1,
        input);

    if (
        file_header.signature != 0x4D42 ||
        info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf(
            "Error: Only uncompressed "
            "24-bit BMP files are supported.\n");

        fclose(input);
        fclose(output);

        return 1;
    }

    int message_length = 0;

    while (
        message[message_length] != '\0')
    {
        message_length++;
    }

    if (message_length == 0)
    {
        printf(
            "Error: Message cannot be empty.\n");

        fclose(input);
        fclose(output);

        return 1;
    }

    int capacity =
        calculate_capacity(input_filename);

    if (message_length > capacity)
    {
        printf(
            "Error: Message is too large "
            "for this image.\n");

        fclose(input);
        fclose(output);

        return 1;
    }

    /*
     * Copy everything before pixel data.
     */
    fseek(
        input,
        0,
        SEEK_SET);

    unsigned char byte;

    for (
        uint32_t i = 0;
        i < file_header.pixel_data_offset;
        i++)
    {
        fread(&byte, 1, 1, input);

        fwrite(&byte, 1, 1, output);
    }

    /*
     * Store message length.
     *
     * 32 bits.
     */
    uint32_t length =
        (uint32_t)message_length;

    for (int i = 31; i >= 0; i--)
    {
        fread(
            &byte,
            1,
            1,
            input);

        unsigned char bit =
            (length >> i) & 1;

        byte =
            (byte & 0xFE) | bit;

        fwrite(
            &byte,
            1,
            1,
            output);
    }

    /*
     * Store actual message.
     */
    int message_index = 0;

    int bit_index = 0;

    for (
        int i = 0;
        i < message_length * 8;
        i++)
    {
        fread(
            &byte,
            1,
            1,
            input);

        unsigned char bit =
            (message[message_index] >>
             (7 - bit_index)) &
            1;

        byte =
            (byte & 0xFE) | bit;

        fwrite(
            &byte,
            1,
            1,
            output);

        bit_index++;

        if (bit_index == 8)
        {
            bit_index = 0;

            message_index++;
        }
    }

    /*
     * Copy remaining image bytes.
     */
    while (
        fread(&byte, 1, 1, input) == 1)
    {
        fwrite(
            &byte,
            1,
            1,
            output);
    }

    fclose(input);
    fclose(output);

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - HIDE\n");
    printf("========================================\n\n");

    printf(
        "Input Image  : %s\n",
        input_filename);

    printf(
        "Output Image : %s\n",
        output_filename);

    printf(
        "Message      : %s\n",
        message);

    printf(
        "Message Size : %d bytes\n",
        message_length);

    printf(
        "\nMessage successfully hidden!\n");

    printf(
        "Output saved to: %s\n",
        output_filename);

    printf(
        "\n========================================\n");

    return 0;
}

/*
 * Hide a binary file inside a BMP.
 */
int hide_file(
    const char *input_image,
    const char *output_image,
    const char *payload_file,
    int payload_type)
{
    if (
        input_image == NULL ||
        output_image == NULL ||
        payload_file == NULL)
    {
        printf(
            "Error: Invalid input parameters.\n");

        return 1;
    }

    if (
        strcmp(
            input_image,
            output_image) == 0)
    {
        printf(
            "Error: Input and output images "
            "must be different.\n");

        return 1;
    }

    /*
     * Open payload.
     */
    FILE *payload =
        fopen(payload_file, "rb");

    if (payload == NULL)
    {
        printf(
            "Error: Could not open payload file.\n");

        return 1;
    }

    /*
     * Determine payload size.
     */
    fseek(
        payload,
        0,
        SEEK_END);

    long payload_size =
        ftell(payload);

    fseek(
        payload,
        0,
        SEEK_SET);

    if (payload_size < 0)
    {
        printf(
            "Error: Could not determine "
            "payload size.\n");

        fclose(payload);

        return 1;
    }

    /*
     * Get filename only.
     */
    const char *filename =
        strrchr(payload_file, '\\');

    if (filename != NULL)
    {
        filename++;
    }
    else
    {
        filename =
            strrchr(payload_file, '/');

        if (filename != NULL)
        {
            filename++;
        }
        else
        {
            filename =
                payload_file;
        }
    }

    size_t filename_length =
        strlen(filename);

    if (filename_length > 65535)
    {
        printf(
            "Error: Filename is too long.\n");

        fclose(payload);

        return 1;
    }

    /*
     * Calculate checksum.
     */
    uint64_t checksum =
        calculate_file_checksum(
            payload_file);

    if (checksum == 0)
    {
        printf(
            "Error: Could not calculate "
            "payload checksum.\n");

        fclose(payload);

        return 1;
    }

    /*
     * Open input BMP.
     */
    FILE *input =
        fopen(input_image, "rb");

    if (input == NULL)
    {
        printf(
            "Error: Could not open input image.\n");

        fclose(payload);

        return 1;
    }

    /*
     * Open output BMP.
     */
    FILE *output =
        fopen(output_image, "wb");

    if (output == NULL)
    {
        printf(
            "Error: Could not create output image.\n");

        fclose(payload);
        fclose(input);

        return 1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    if (
        fread(
            &file_header,
            sizeof(BMPFileHeader),
            1,
            input) != 1 ||
        fread(
            &info_header,
            sizeof(BMPInfoHeader),
            1,
            input) != 1)
    {
        printf(
            "Error: Could not read BMP header.\n");

        fclose(payload);
        fclose(input);
        fclose(output);

        return 1;
    }

    /*
     * Validate BMP.
     */
    if (
        file_header.signature != 0x4D42 ||
        info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf(
            "Error: Only uncompressed "
            "24-bit BMP files are supported.\n");

        fclose(payload);
        fclose(input);
        fclose(output);

        return 1;
    }

    /*
     * Calculate carrier capacity.
     */
    int width =
        info_header.width;

    int height =
        info_header.height;

    int bytes_per_row =
        width * 3;

    int padding =
        (4 - (bytes_per_row % 4)) % 4;

    long total_pixel_bytes =
        (long)(bytes_per_row + padding) * height;

    long capacity =
        (total_pixel_bytes / 8);

    /*
     * Required bytes:
     *
     * header
     * filename
     * payload
     */
    long required_bytes =
        PAYLOAD_HEADER_SIZE +
        (long)filename_length +
        payload_size;

    if (required_bytes > capacity)
    {
        printf("\n");
        printf(
            "Error: Payload is too large "
            "for this image.\n");

        printf(
            "Required : %ld bytes\n",
            required_bytes);

        printf(
            "Capacity : %ld bytes\n",
            capacity);

        fclose(payload);
        fclose(input);
        fclose(output);

        return 1;
    }

    /*
     * Copy BMP header.
     */
    fseek(
        input,
        0,
        SEEK_SET);

    unsigned char byte;

    for (
        uint32_t i = 0;
        i < file_header.pixel_data_offset;
        i++)
    {
        fread(
            &byte,
            1,
            1,
            input);

        fwrite(
            &byte,
            1,
            1,
            output);
    }

    /*
     * Create metadata header.
     */
    unsigned char header[PAYLOAD_HEADER_SIZE];

    write_payload_header(
        header,
        (uint8_t)payload_type,
        (uint16_t)filename_length,
        (uint64_t)payload_size,
        checksum);

    /*
     * Helper variables for bit encoding.
     */
    unsigned char current_byte;

    /*
     * Macro-like local logic is avoided.
     *
     * First encode the header.
     */
    for (
        int i = 0;
        i < PAYLOAD_HEADER_SIZE;
        i++)
    {
        for (
            int bit = 7;
            bit >= 0;
            bit--)
        {
            if (
                fread(
                    &current_byte,
                    1,
                    1,
                    input) != 1)
            {
                printf(
                    "Error: Unexpected end "
                    "of image data.\n");

                fclose(payload);
                fclose(input);
                fclose(output);

                return 1;
            }

            unsigned char data_bit =
                (header[i] >> bit) & 1;

            current_byte =
                (current_byte & 0xFE) |
                data_bit;

            fwrite(
                &current_byte,
                1,
                1,
                output);
        }
    }

    /*
     * Encode filename.
     */
    for (
        size_t i = 0;
        i < filename_length;
        i++)
    {
        for (
            int bit = 7;
            bit >= 0;
            bit--)
        {
            fread(
                &current_byte,
                1,
                1,
                input);

            unsigned char data_bit =
                (filename[i] >> bit) & 1;

            current_byte =
                (current_byte & 0xFE) |
                data_bit;

            fwrite(
                &current_byte,
                1,
                1,
                output);
        }
    }

    /*
     * Encode payload file.
     */
    unsigned char payload_buffer[4096];

    size_t bytes_read;

    while (
        (
            bytes_read =
                fread(
                    payload_buffer,
                    1,
                    sizeof(payload_buffer),
                    payload)) > 0)
    {
        for (
            size_t i = 0;
            i < bytes_read;
            i++)
        {
            for (
                int bit = 7;
                bit >= 0;
                bit--)
            {
                if (
                    fread(
                        &current_byte,
                        1,
                        1,
                        input) != 1)
                {
                    printf(
                        "Error: Image capacity "
                        "ended unexpectedly.\n");

                    fclose(payload);
                    fclose(input);
                    fclose(output);

                    return 1;
                }

                unsigned char data_bit =
                    (payload_buffer[i] >>
                     bit) &
                    1;

                current_byte =
                    (current_byte & 0xFE) |
                    data_bit;

                fwrite(
                    &current_byte,
                    1,
                    1,
                    output);
            }
        }
    }

    /*
     * Copy remaining image bytes.
     */
    while (
        fread(
            &byte,
            1,
            1,
            input) == 1)
    {
        fwrite(
            &byte,
            1,
            1,
            output);
    }

    fclose(payload);
    fclose(input);
    fclose(output);

    const char *type_name;

    if (payload_type == PAYLOAD_TYPE_TEXT)
    {
        type_name = "TEXT";
    }
    else if (payload_type == PAYLOAD_TYPE_IMAGE)
    {
        type_name = "IMAGE";
    }
    else
    {
        type_name = "AUDIO";
    }

    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - FILE HIDE\n");
    printf("========================================\n\n");

    printf(
        "Carrier Image : %s\n",
        input_image);

    printf(
        "Output Image  : %s\n",
        output_image);

    printf(
        "Payload       : %s\n",
        payload_file);

    printf(
        "Payload Type  : %s\n",
        type_name);

    printf(
        "Payload Size  : %ld bytes\n",
        payload_size);

    printf(
        "\nFile successfully hidden!\n");

    printf(
        "Output saved to: %s\n",
        output_image);

    printf(
        "\n========================================\n");

    return 0;
}