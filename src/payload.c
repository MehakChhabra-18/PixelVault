#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "payload.h"


/*
 * Simple 64-bit FNV-1a checksum.
 *
 * This is not encryption.
 * It is used only to verify that the
 * extracted file matches the embedded data.
 */
uint64_t calculate_file_checksum(
    const char *filename
)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        return 0;
    }

    uint64_t hash = 1469598103934665603ULL;

    unsigned char buffer[4096];

    size_t bytes_read;

    while (
        (bytes_read =
            fread(
                buffer,
                1,
                sizeof(buffer),
                file
            )) > 0
    )
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            hash ^= buffer[i];

            hash *= 1099511628211ULL;
        }
    }

    fclose(file);

    return hash;
}


/*
 * Determine payload type.
 */
int detect_payload_type(
    const char *filename
)
{
    const char *extension =
        strrchr(filename, '.');

    if (extension == NULL)
    {
        return 0;
    }


    /*
     * TEXT
     */
    if (
        strcmp(extension, ".txt") == 0 ||
        strcmp(extension, ".TXT") == 0
    )
    {
        return PAYLOAD_TYPE_TEXT;
    }


    /*
     * IMAGE
     */
    if (
        strcmp(extension, ".jpg") == 0 ||
        strcmp(extension, ".JPG") == 0 ||
        strcmp(extension, ".jpeg") == 0 ||
        strcmp(extension, ".JPEG") == 0 ||
        strcmp(extension, ".png") == 0 ||
        strcmp(extension, ".PNG") == 0
    )
    {
        return PAYLOAD_TYPE_IMAGE;
    }


    /*
     * AUDIO
     */
    if (
        strcmp(extension, ".wav") == 0 ||
        strcmp(extension, ".WAV") == 0
    )
    {
        return PAYLOAD_TYPE_AUDIO;
    }


    return 0;
}


/*
 * Write metadata into a byte buffer.
 *
 * We manually serialize the fields instead of
 * writing a C struct directly. This avoids
 * compiler padding/alignment problems.
 */
int write_payload_header(
    unsigned char *buffer,
    uint8_t type,
    uint16_t filename_length,
    uint64_t file_size,
    uint64_t checksum
)
{
    if (buffer == NULL)
    {
        return 1;
    }


    /*
     * Magic signature.
     *
     * 8 bytes.
     */
    memset(
        buffer,
        0,
        PAYLOAD_HEADER_SIZE
    );

    memcpy(
        buffer,
        PAYLOAD_MAGIC,
        strlen(PAYLOAD_MAGIC)
    );


    /*
     * Payload type.
     */
    buffer[8] = type;


    /*
     * Filename length.
     *
     * Big-endian.
     */
    buffer[9] =
        (filename_length >> 8) & 0xFF;

    buffer[10] =
        filename_length & 0xFF;


    /*
     * File size.
     *
     * 8 bytes.
     */
    for (int i = 0; i < 8; i++)
    {
        buffer[11 + i] =
            (file_size >>
             (56 - (i * 8))) & 0xFF;
    }


    /*
     * Checksum.
     *
     * 8 bytes.
     */
    for (int i = 0; i < 8; i++)
    {
        buffer[19 + i] =
            (checksum >>
             (56 - (i * 8))) & 0xFF;
    }


    return 0;
}


/*
 * Read and validate payload header.
 */
int read_payload_header(
    const unsigned char *buffer,
    PayloadMetadata *metadata
)
{
    if (
        buffer == NULL ||
        metadata == NULL
    )
    {
        return 1;
    }


    /*
     * Validate magic.
     */
    if (
        memcmp(
            buffer,
            PAYLOAD_MAGIC,
            strlen(PAYLOAD_MAGIC)
        ) != 0
    )
    {
        return 1;
    }


    /*
     * Payload type.
     */
    metadata->type = buffer[8];


    if (
        metadata->type != PAYLOAD_TYPE_TEXT &&
        metadata->type != PAYLOAD_TYPE_IMAGE &&
        metadata->type != PAYLOAD_TYPE_AUDIO
    )
    {
        return 1;
    }


    /*
     * Filename length.
     */
    metadata->filename_length =
        ((uint16_t)buffer[9] << 8) |
        buffer[10];


    /*
     * File size.
     */
    metadata->file_size = 0;

    for (int i = 0; i < 8; i++)
    {
        metadata->file_size <<= 8;

        metadata->file_size |=
            buffer[11 + i];
    }


    /*
     * Checksum.
     */
    metadata->checksum = 0;

    for (int i = 0; i < 8; i++)
    {
        metadata->checksum <<= 8;

        metadata->checksum |=
            buffer[19 + i];
    }


    return 0;
}