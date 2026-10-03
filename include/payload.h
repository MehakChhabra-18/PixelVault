#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdint.h>

#define PAYLOAD_MAGIC "PVLT001"
#define PAYLOAD_MAGIC_SIZE 8

#define PAYLOAD_TYPE_TEXT  1
#define PAYLOAD_TYPE_IMAGE 2
#define PAYLOAD_TYPE_AUDIO 3

#define PAYLOAD_HEADER_SIZE 27

typedef struct
{
    uint8_t type;
    uint16_t filename_length;
    uint64_t file_size;
    uint64_t checksum;

} PayloadMetadata;


/*
 * Calculate checksum of a file.
 */
uint64_t calculate_file_checksum(
    const char *filename
);


/*
 * Determine payload type from filename.
 */
int detect_payload_type(
    const char *filename
);


/*
 * Write PixelVault payload header.
 */
int write_payload_header(
    unsigned char *buffer,
    uint8_t type,
    uint16_t filename_length,
    uint64_t file_size,
    uint64_t checksum
);


/*
 * Read PixelVault payload header.
 */
int read_payload_header(
    const unsigned char *buffer,
    PayloadMetadata *metadata
);

#endif