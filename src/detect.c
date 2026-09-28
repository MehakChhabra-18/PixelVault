#include <stdio.h>

#include "bmp.h"
#include "detect.h"


int detect_steganography(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error: Could not open %s\n", filename);
        return 1;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    if (fread(&file_header,
              sizeof(BMPFileHeader),
              1,
              file) != 1)
    {
        printf("Error: Could not read BMP header.\n");
        fclose(file);
        return 1;
    }

    if (fread(&info_header,
              sizeof(BMPInfoHeader),
              1,
              file) != 1)
    {
        printf("Error: Could not read BMP information.\n");
        fclose(file);
        return 1;
    }


    /*
     * Validate BMP.
     */
    if (file_header.signature != 0x4D42)
    {
        printf("Error: Not a valid BMP file.\n");
        fclose(file);
        return 1;
    }

    if (info_header.bits_per_pixel != 24 ||
        info_header.compression != 0)
    {
        printf(
            "Error: Only uncompressed 24-bit BMP files "
            "are supported.\n"
        );

        fclose(file);
        return 1;
    }


    int width = info_header.width;
    int height = info_header.height;

    int bytes_per_row = width * 3;

    int padding =
        (4 - (bytes_per_row % 4)) % 4;


    /*
     * Move to pixel data.
     */
    fseek(
        file,
        file_header.pixel_data_offset,
        SEEK_SET
    );


    long total_bytes = 0;
    long zero_count = 0;
    long one_count = 0;

    long transitions = 0;

    int previous_lsb = -1;


    /*
     * Analyze every actual pixel byte.
     */
    for (int row = 0;
         row < height;
         row++)
    {
        for (int column = 0;
             column < bytes_per_row;
             column++)
        {
            unsigned char byte;

            if (fread(&byte, 1, 1, file) != 1)
            {
                printf("Error: Could not read pixel data.\n");

                fclose(file);
                return 1;
            }

            int current_lsb = byte & 1;

            total_bytes++;

            if (current_lsb == 0)
            {
                zero_count++;
            }
            else
            {
                one_count++;
            }


            /*
             * Count transitions between
             * consecutive LSBs.
             */
            if (previous_lsb != -1 &&
                previous_lsb != current_lsb)
            {
                transitions++;
            }

            previous_lsb = current_lsb;
        }


        /*
         * Skip BMP padding.
         */
        for (int p = 0;
             p < padding;
             p++)
        {
            unsigned char padding_byte;

            fread(&padding_byte, 1, 1, file);
        }
    }


    fclose(file);


    /*
     * Calculate percentages.
     */
    double zero_percentage =
        ((double)zero_count /
         (double)total_bytes) * 100.0;

    double one_percentage =
        ((double)one_count /
         (double)total_bytes) * 100.0;


    /*
     * Transition rate.
     */
    double transition_rate =
        ((double)transitions /
         (double)(total_bytes - 1)) * 100.0;


    /*
     * Difference between zero and one distribution.
     */
    double imbalance =
        zero_percentage > one_percentage
        ? zero_percentage - one_percentage
        : one_percentage - zero_percentage;


    /*
     * Very simple indicators.
     *
     * These are NOT proof of steganography.
     */
    int imbalance_indicator = 0;
    int transition_indicator = 0;


    if (imbalance > 20.0)
    {
        imbalance_indicator = 1;
    }


    /*
     * Very low or very high transition rates
     * can be worth investigating.
     */
    if (transition_rate < 10.0 ||
        transition_rate > 90.0)
    {
        transition_indicator = 1;
    }


    int indicators =
        imbalance_indicator +
        transition_indicator;


    /*
     * Display report.
     */
    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - DETECTION\n");
    printf("========================================\n\n");

    printf("Image: %s\n\n", filename);


    printf("LSB Statistics\n");
    printf("----------------------------------------\n");

    printf("Total Pixel Bytes : %ld\n",
           total_bytes);

    printf("LSB = 0           : %ld\n",
           zero_count);

    printf("LSB = 1           : %ld\n",
           one_count);

    printf("LSB 0 Percentage  : %.2f%%\n",
           zero_percentage);

    printf("LSB 1 Percentage  : %.2f%%\n",
           one_percentage);


    printf("\nLSB Pattern Analysis\n");
    printf("----------------------------------------\n");

    printf("LSB Transitions   : %ld\n",
           transitions);

    printf("Transition Rate   : %.2f%%\n",
           transition_rate);


    printf("\nIndicators\n");
    printf("----------------------------------------\n");

    if (imbalance_indicator)
    {
        printf(
            "LSB distribution  : UNUSUAL\n"
        );
    }
    else
    {
        printf(
            "LSB distribution  : NORMAL RANGE\n"
        );
    }


    if (transition_indicator)
    {
        printf(
            "LSB transitions   : UNUSUAL\n"
        );
    }
    else
    {
        printf(
            "LSB transitions   : NORMAL RANGE\n"
        );
    }


    printf("\nAssessment\n");
    printf("----------------------------------------\n");

    if (indicators == 0)
    {
        printf(
            "No strong statistical indicators found.\n"
        );
    }
    else if (indicators == 1)
    {
        printf(
            "One statistical indicator requires "
            "further investigation.\n"
        );
    }
    else
    {
        printf(
            "Multiple statistical indicators "
            "require investigation.\n"
        );
    }


    printf(
        "\nImportant: Statistical indicators do NOT "
        "prove that hidden data exists.\n"
    );


    printf("\n========================================\n");

    return 0;
}