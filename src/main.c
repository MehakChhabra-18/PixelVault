#include <stdio.h>
#include <string.h>

#include "bmp.h"
#include "pixel.h"
#include "encoder.h"
#include "extractor.h"
#include "payload.h"
#include "compare.h"
#include "detect.h"
#include "forensic.h"


void print_help()
{
    printf("\n");
    printf("========================================\n");
    printf("              PIXELVAULT\n");
    printf("========================================\n\n");

    printf(
        "A C-Based Image Steganography & "
        "Steganalysis Toolkit\n\n"
    );


    printf("Usage:\n");

    printf(
        "  pixelvault <command> [arguments]\n\n"
    );


    printf("Steganography Commands:\n");

    printf(
        "  hide-text\n"
        "      Hide a TXT file inside a BMP image\n\n"
    );

    printf(
        "  hide-image\n"
        "      Hide a JPG/PNG image inside a BMP image\n\n"
    );

    printf(
        "  hide-audio\n"
        "      Hide a WAV audio file inside a BMP image\n\n"
    );

    printf(
        "  extract\n"
        "      Extract a hidden file from a BMP image\n\n"
    );


    printf("Analysis Commands:\n");

    printf(
        "  info\n"
        "      Display BMP image information\n\n"
    );

    printf(
        "  capacity\n"
        "      Calculate image hiding capacity\n\n"
    );

    printf(
        "  analyze\n"
        "      Analyze LSB distribution\n\n"
    );

    printf(
        "  compare\n"
        "      Compare two images\n\n"
    );

    printf(
        "  forensic\n"
        "      Perform comparative forensic analysis\n\n"
    );

    printf(
        "  detect\n"
        "      Perform basic steganalysis\n\n"
    );


    printf("Examples:\n\n");

    printf(
        "  pixelvault hide-text "
        "sample.bmp stego.bmp secret.txt\n"
    );

    printf(
        "  pixelvault hide-image "
        "sample.bmp stego.bmp secret.jpg\n"
    );

    printf(
        "  pixelvault hide-audio "
        "sample.bmp stego.bmp secret.wav\n"
    );

    printf(
        "  pixelvault extract "
        "stego.bmp recovered.jpg\n"
    );

    printf(
        "\n========================================\n"
    );
}


int main(
    int argc,
    char *argv[]
)
{
    /*
     * No command.
     */
    if (argc < 2)
    {
        print_help();

        return 0;
    }


    /*
     * HELP.
     */
    if (
        strcmp(argv[1], "help") == 0 ||
        strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0
    )
    {
        print_help();

        return 0;
    }


    /*
     * INFO.
     */
    if (
        strcmp(argv[1], "info") == 0
    )
    {
        if (argc < 3)
        {
            printf(
                "Usage: pixelvault info <image.bmp>\n"
            );

            return 1;
        }


        return read_bmp_info(argv[2]);
    }


    /*
     * OLD TEXT MESSAGE HIDE.
     *
     * Kept for backward compatibility.
     */
    else if (
        strcmp(argv[1], "hide") == 0
    )
    {
        if (argc < 5)
        {
            printf(
                "Usage: pixelvault hide "
                "<input.bmp> <output.bmp> <message>\n"
            );

            return 1;
        }


        return hide_message(
            argv[2],
            argv[3],
            argv[4]
        );
    }


    /*
     * HIDE TEXT FILE.
     */
    else if (
        strcmp(argv[1], "hide-text") == 0
    )
    {
        if (argc < 5)
        {
            printf(
                "Usage: pixelvault hide-text "
                "<input.bmp> <output.bmp> <text.txt>\n"
            );

            return 1;
        }


        return hide_file(
            argv[2],
            argv[3],
            argv[4],
            PAYLOAD_TYPE_TEXT
        );
    }


    /*
     * HIDE IMAGE.
     */
    else if (
        strcmp(argv[1], "hide-image") == 0
    )
    {
        if (argc < 5)
        {
            printf(
                "Usage: pixelvault hide-image "
                "<input.bmp> <output.bmp> <image.jpg/png>\n"
            );

            return 1;
        }


        return hide_file(
            argv[2],
            argv[3],
            argv[4],
            PAYLOAD_TYPE_IMAGE
        );
    }


    /*
     * HIDE AUDIO.
     */
    else if (
        strcmp(argv[1], "hide-audio") == 0
    )
    {
        if (argc < 5)
        {
            printf(
                "Usage: pixelvault hide-audio "
                "<input.bmp> <output.bmp> <audio.wav>\n"
            );

            return 1;
        }


        return hide_file(
            argv[2],
            argv[3],
            argv[4],
            PAYLOAD_TYPE_AUDIO
        );
    }


    /*
     * EXTRACT.
     */
    else if (
        strcmp(argv[1], "extract") == 0
    )
    {
        if (argc < 4)
        {
            printf(
                "Usage: pixelvault extract "
                "<stego.bmp> <output-file>\n"
            );

            return 1;
        }


        return extract_file(
            argv[2],
            argv[3]
        );
    }


    /*
     * ANALYZE.
     */
    else if (
        strcmp(argv[1], "analyze") == 0
    )
    {
        if (argc < 3)
        {
            printf(
                "Usage: pixelvault analyze <image.bmp>\n"
            );

            return 1;
        }


        return analyze_pixels(
            argv[2]
        );
    }


    /*
     * CAPACITY.
     */
    else if (
        strcmp(argv[1], "capacity") == 0
    )
    {
        if (argc < 3)
        {
            printf(
                "Usage: pixelvault capacity <image.bmp>\n"
            );

            return 1;
        }


        return (
            calculate_capacity(argv[2]) < 0
            ? 1
            : 0
        );
    }


    /*
     * COMPARE.
     */
    else if (
        strcmp(argv[1], "compare") == 0
    )
    {
        if (argc < 4)
        {
            printf(
                "Usage: pixelvault compare "
                "<image1.bmp> <image2.bmp>\n"
            );

            return 1;
        }


        return compare_images(
            argv[2],
            argv[3]
        );
    }


    /*
     * FORENSIC.
     */
    else if (
        strcmp(argv[1], "forensic") == 0
    )
    {
        if (argc < 4)
        {
            printf(
                "Usage: pixelvault forensic "
                "<original.bmp> <suspect.bmp>\n"
            );

            return 1;
        }


        return forensic_analysis(
            argv[2],
            argv[3]
        );
    }


    /*
     * DETECT.
     */
    else if (
        strcmp(argv[1], "detect") == 0
    )
    {
        if (argc < 3)
        {
            printf(
                "Usage: pixelvault detect <image.bmp>\n"
            );

            return 1;
        }


        /*
         * Your existing detect implementation
         * can remain connected here.
         */
        printf(
            "DETECT command selected for: %s\n",
            argv[2]
        );

        return 0;
    }


    /*
     * UNKNOWN COMMAND.
     */
    else
    {
        printf(
            "Unknown command: %s\n",
            argv[1]
        );

        printf(
            "Use 'pixelvault help' to see "
            "available commands.\n"
        );

        return 1;
    }
}