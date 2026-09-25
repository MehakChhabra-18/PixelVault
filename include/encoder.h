#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

int calculate_capacity(const char *filename);

int hide_message(
    const char *input_filename,
    const char *output_filename,
    const char *message
);

int extract_message(
    const char *filename
);

#endif