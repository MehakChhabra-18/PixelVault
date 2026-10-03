#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "bmp.h"
#include "payload.h"
#include "extractor.h"

/*
 * Read one byte from the LSB stream.
 */
static int read_lsb_bit(
    FILE *file,
    unsigned char *bit)
{
    unsigned char byte;

    if (
        fread(
            &byte,
            1,
            1,
            file) != 1)
    {
        return 1;
    }

    *bit = byte & 1;

    return 0;
}

/*
 * Read one complete byte from LSB stream.
 */
static int read_lsb_byte(
    FILE *file,
    unsigned char *result)
{
    unsigned char value = 0;

    for (int bit = 7; bit >= 0; bit--)
    {
        unsigned char lsb;

        if (
            read_lsb_bit(
                file,
                &lsb) != 0)
        {
            return 1;
        }

        value |=
            (lsb << bit);
    }

    *result = value;

    return 0;
}

/*
 * Extract binary payload.
 */
int extract_file(
    const char *stego_image,
    const char *output_file)
{
    FILE *input =
        fopen(stego_image, "rb");

    if (input == NULL)
    {
        printf(
            "Error: Could not open stego image.\n");

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

        fclose(input);

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

        fclose(input);

        return 1;
    }

    /*
     * Move to pixel data.
     */
    fseek(
        input,
        file_header.pixel_data_offset,
        SEEK_SET);

    /*
     * Read PixelVault header.
     */
    unsigned char header[PAYLOAD_HEADER_SIZE];

    for (
        int i = 0;
        i < PAYLOAD_HEADER_SIZE;
        i++)
    {
        if (
            read_lsb_byte(
                input,
                &header[i]) != 0)
        {
            printf(
                "Error: Could not read "
                "PixelVault payload header.\n");

            fclose(input);

            return 1;
        }
    }

    /*
     * Parse header.
     */
    PayloadMetadata metadata;

    if (
        read_payload_header(
            header,
            &metadata) != 0)
    {
        printf(
            "Error: No valid PixelVault "
            "payload found in this image.\n");

        fclose(input);

        return 1;
    }

    /*
     * Read filename.
     */
    if (
        metadata.filename_length == 0 ||
        metadata.filename_length > 65535)
    {
        printf(
            "Error: Invalid payload filename length.\n");

        fclose(input);

        return 1;
    }

    char *filename =
        malloc(
            metadata.filename_length + 1);

    if (filename == NULL)
    {
        printf(
            "Error: Memory allocation failed.\n");

        fclose(input);

        return 1;
    }

    for (
        uint16_t i = 0;
        i < metadata.filename_length;
        i++)
    {
        if (
            read_lsb_byte(
                input,
                (unsigned char *)&filename[i]) != 0)
        {
            printf(
                "Error: Could not read payload filename.\n");

            free(filename);
            fclose(input);

            return 1;
        }
    }

    filename[metadata.filename_length] = '\0';

    /*
     * Display payload information.
     */
    const char *type_name;

    if (
        metadata.type ==
        PAYLOAD_TYPE_TEXT)
    {
        type_name = "TEXT";
    }
    else if (
        metadata.type ==
        PAYLOAD_TYPE_IMAGE)
    {
        type_name = "IMAGE";
    }
    else
    {
        type_name = "AUDIO";
    }

    printf("\n");
    printf("========================================\n");
    printf("      PIXELVAULT - FILE EXTRACT\n");
    printf("========================================\n\n");

    printf(
        "Stego Image   : %s\n",
        stego_image);

    printf(
        "Payload Type  : %s\n",
        type_name);

    printf(
        "Original Name : %s\n",
        filename);

    printf(
        "Payload Size  : %llu bytes\n",
        (unsigned long long)
            metadata.file_size);

    /*
     * Open output file.
     */
    FILE *output =
        fopen(output_file, "wb");

    if (output == NULL)
    {
        printf(
            "Error: Could not create output file.\n");

        free(filename);
        fclose(input);

        return 1;
    }

    /*
     * Extract payload.
     */
    unsigned char buffer[4096];

    uint64_t remaining =
        metadata.file_size;

    while (remaining > 0)
    {
        size_t chunk =
            remaining > sizeof(buffer)
                ? sizeof(buffer)
                : (size_t)remaining;

        for (
            size_t i = 0;
            i < chunk;
            i++)
        {
            if (
                read_lsb_byte(
                    input,
                    &buffer[i]) != 0)
            {
                printf(
                    "Error: Image ended before "
                    "payload was completely extracted.\n");

                fclose(output);
                free(filename);
                fclose(input);

                remove(output_file);

                return 1;
            }
        }

        fwrite(
            buffer,
            1,
            chunk,
            output);

        remaining -= chunk;
    }

    fclose(output);

    /*
     * Verify extracted file.
     */
    uint64_t extracted_checksum =
        calculate_file_checksum(
            output_file);

    if (
        extracted_checksum !=
        metadata.checksum)
    {
        printf(
            "\nWARNING: Checksum verification failed.\n");

        printf(
            "The extracted file may be corrupted.\n");

        free(filename);
        fclose(input);

        return 1;
    }

    printf(
        "\nChecksum verified successfully.\n");

    printf(
        "File successfully extracted!\n");
    printf(
        "Output saved to: %s\n",
        output_file);

    printf(
        "\n========================================\n");

    free(filename);
    fclose(input);

    return 0;
}