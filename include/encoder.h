#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

int calculate_capacity(
    const char *filename
);

int hide_message(
    const char *input_filename,
    const char *output_filename,
    const char *message
);


/*
 * Hide a file inside a BMP image.
 *
 * payload_type:
 *
 * 1 = TEXT
 * 2 = IMAGE
 * 3 = AUDIO
 */
int hide_file(
    const char *input_image,
    const char *output_image,
    const char *payload_file,
    int payload_type
);

#endif