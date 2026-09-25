#include <stdio.h>
#include <string.h>

#include "bmp.h"
#include "pixel.h"
#include "encoder.h"
#include "compare.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("PixelVault\n");
        printf("A C-Based Image Steganography & Steganalysis Toolkit\n\n");

        printf("Usage:\n");
        printf("  pixelvault <command>\n\n");

        printf("Commands:\n");
        printf("  info      Display image information\n");
        printf("  hide      Hide a message inside an image\n");
        printf("  extract   Extract a hidden message\n");
        printf("  capacity  Calculate image hiding capacity\n");
        printf("  compare   Compare two images\n");
        printf("  analyze   Analyze image properties\n");
        printf("  detect    Perform basic steganalysis\n");

        return 0;
    }

    /* INFO */
    if (strcmp(argv[1], "info") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: pixelvault info <image.bmp>\n");
            return 1;
        }

        return read_bmp_info(argv[2]);
    }

    /* HIDE */
    else if (strcmp(argv[1], "hide") == 0)
    {
        if (argc < 5)
        {
            printf("Usage: pixelvault hide <input.bmp> <output.bmp> <message>\n");
            return 1;
        }

        return hide_message(
            argv[2],
            argv[3],
            argv[4]
        );
    }

    /* EXTRACT */
    else if (strcmp(argv[1], "extract") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: pixelvault extract <image.bmp>\n");
            return 1;
        }

        return extract_message(argv[2]);
    }

    /* CAPACITY */
    else if (strcmp(argv[1], "capacity") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: pixelvault capacity <image.bmp>\n");
            return 1;
        }

        return calculate_capacity(argv[2]);
    }

    /* COMPARE */
    else if (strcmp(argv[1], "compare") == 0)
    {
        if(argc<4)
        {
            printf("Usage: pixelvault compare <image1.bmp> <image2.bmp>\n");
            return 1;
        }

        return compare_images(argv[2],argv[3]);
    }

    /* ANALYZE */
    else if (strcmp(argv[1], "analyze") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: pixelvault analyze <image.bmp>\n");
            return 1;
        }

        return analyze_pixels(argv[2]);
    }

    /* DETECT */
    else if (strcmp(argv[1], "detect") == 0)
    {
        printf("DETECT command selected.\n");
    }

    /* UNKNOWN */
    else
    {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}