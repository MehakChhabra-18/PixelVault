#include <stdio.h>

#include "bmp.h"
#include "pixel.h"


int analyze_pixels(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error: Could not open %s\n", filename);
        return 1;
    }


    /*
     * Read BMP headers.
     */
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    if (fread(&file_header,
              sizeof(BMPFileHeader),
              1,
              file) != 1)
    {
        printf("Error: Could not read BMP file header.\n");
        fclose(file);
        return 1;
    }

    if (fread(&info_header,
              sizeof(BMPInfoHeader),
              1,
              file) != 1)
    {
        printf("Error: Could not read BMP information header.\n");
        fclose(file);
        return 1;
    }


    /*
     * Validate BMP signature.
     */
    if (file_header.signature != 0x4D42)
    {
        printf("Error: Not a valid BMP file.\n");
        fclose(file);
        return 1;
    }


    /*
     * PixelVault currently supports
     * uncompressed 24-bit BMP files.
     */
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


    /*
     * Move to the beginning of pixel data.
     */
    if (fseek(file,
              file_header.pixel_data_offset,
              SEEK_SET) != 0)
    {
        printf("Error: Could not locate pixel data.\n");
        fclose(file);
        return 1;
    }


    /*
     * BMP rows are padded to a multiple of 4 bytes.
     */
    int bytes_per_row = width * 3;

    int padding =
        (4 - (bytes_per_row % 4)) % 4;


    /*
     * Statistics.
     */
    long total_pixel_bytes = 0;

    long lsb_zero = 0;
    long lsb_one = 0;


    /*
     * Analyze every row.
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
                printf(
                    "Error: Could not read pixel data.\n"
                );

                fclose(file);
                return 1;
            }


            total_pixel_bytes++;


            /*
             * Extract the Least Significant Bit.
             */
            if ((byte & 1) == 0)
            {
                lsb_zero++;
            }
            else
            {
                lsb_one++;
            }
        }


        /*
         * Skip BMP row padding.
         *
         * Padding bytes are not actual pixel data.
         */
        for (int p = 0;
             p < padding;
             p++)
        {
            unsigned char padding_byte;

            fread(&padding_byte, 1, 1, file);
        }
    }


    /*
     * Calculate percentages.
     */
    double zero_percentage = 0.0;
    double one_percentage = 0.0;

    if (total_pixel_bytes > 0)
    {
        zero_percentage =
            ((double)lsb_zero /
             (double)total_pixel_bytes) * 100.0;

        one_percentage =
            ((double)lsb_one /
             (double)total_pixel_bytes) * 100.0;
    }


    /*
     * Calculate how balanced the LSBs are.
     *
     * Perfect balance = 50 / 50.
     */
    double lsb_balance =
        100.0 -
        ((zero_percentage > one_percentage
              ? zero_percentage
              : one_percentage) - 50.0) * 2.0;


    if (lsb_balance < 0)
    {
        lsb_balance = 0;
    }


    /*
     * Display report.
     */
    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - STEGANALYSIS\n");
    printf("========================================\n\n");


    printf("Image: %s\n\n", filename);


    printf("Image Properties\n");
    printf("----------------------------------------\n");

    printf("Width             : %d pixels\n", width);
    printf("Height            : %d pixels\n",
           height);

    printf("Bits Per Pixel    : %d\n",
           info_header.bits_per_pixel);

    printf("Pixel Bytes       : %ld\n",
           total_pixel_bytes);


    printf("\nLSB Analysis\n");
    printf("----------------------------------------\n");

    printf("LSB = 0           : %ld\n",
           lsb_zero);

    printf("LSB = 1           : %ld\n",
           lsb_one);

    printf("LSB 0 Percentage   : %.2f%%\n",
           zero_percentage);

    printf("LSB 1 Percentage   : %.2f%%\n",
           one_percentage);

    printf("LSB Balance       : %.2f%%\n",
           lsb_balance);


    /*
     * Basic interpretation.
     *
     * IMPORTANT:
     * This is only an indicator.
     * It does NOT prove steganography.
     */
    printf("\nAnalysis\n");
    printf("----------------------------------------\n");

    if (lsb_balance >= 95.0)
    {
        printf(
            "LSB distribution appears highly balanced.\n"
        );
    }
    else if (lsb_balance >= 85.0)
    {
        printf(
            "LSB distribution appears relatively balanced.\n"
        );
    }
    else
    {
        printf(
            "LSB distribution shows noticeable imbalance.\n"
        );
    }

    printf(
        "\nNote: LSB statistics alone cannot prove "
        "that an image contains hidden data.\n"
    );


    printf("\n========================================\n");


    fclose(file);

    return 0;
}