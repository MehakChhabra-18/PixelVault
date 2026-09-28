#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "compare.h"


int compare_images(
    const char *image1,
    const char *image2
)
{
    FILE *file1 = fopen(image1, "rb");
    FILE *file2 = fopen(image2, "rb");

    if (file1 == NULL)
    {
        printf("Error: Could not open %s\n", image1);
        return 1;
    }

    if (file2 == NULL)
    {
        printf("Error: Could not open %s\n", image2);

        fclose(file1);

        return 1;
    }


    /*
     * Read BMP headers.
     */
    BMPFileHeader header1;
    BMPFileHeader header2;

    BMPInfoHeader info1;
    BMPInfoHeader info2;


    if (fread(&header1,
              sizeof(BMPFileHeader),
              1,
              file1) != 1)
    {
        printf("Error: Could not read first BMP header.\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    if (fread(&info1,
              sizeof(BMPInfoHeader),
              1,
              file1) != 1)
    {
        printf("Error: Could not read first BMP information.\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    if (fread(&header2,
              sizeof(BMPFileHeader),
              1,
              file2) != 1)
    {
        printf("Error: Could not read second BMP header.\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    if (fread(&info2,
              sizeof(BMPInfoHeader),
              1,
              file2) != 1)
    {
        printf("Error: Could not read second BMP information.\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    /*
     * Validate BMP signatures.
     */
    if (header1.signature != 0x4D42 ||
        header2.signature != 0x4D42)
    {
        printf(
            "Error: One or both files are not valid BMP images.\n"
        );

        fclose(file1);
        fclose(file2);

        return 1;
    }


    /*
     * Currently support only uncompressed 24-bit BMP.
     */
    if (info1.bits_per_pixel != 24 ||
        info2.bits_per_pixel != 24 ||
        info1.compression != 0 ||
        info2.compression != 0)
    {
        printf(
            "Error: Only uncompressed 24-bit BMP images "
            "are supported.\n"
        );

        fclose(file1);
        fclose(file2);

        return 1;
    }


    /*
     * Check image dimensions.
     */
    if (info1.width != info2.width ||
        info1.height != info2.height)
    {
        printf("\n");
        printf("========================================\n");
        printf("       PIXELVAULT - IMAGE COMPARE\n");
        printf("========================================\n\n");

        printf("Image A : %s\n", image1);
        printf("Image B : %s\n\n", image2);

        printf("Result\n");
        printf("----------------------------------------\n");

        printf(
            "Images cannot be compared pixel-by-pixel.\n"
        );

        printf(
            "Reason: dimensions are different.\n"
        );

        printf(
            "Image A : %d x %d\n",
            info1.width,
            info1.height
        );

        printf(
            "Image B : %d x %d\n",
            info2.width,
            info2.height
        );

        printf("\n========================================\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    /*
     * Calculate row information.
     *
     * 24-bit BMP = 3 bytes per pixel.
     */
    int bytes_per_row =
        info1.width * 3;

    int padding =
        (4 - (bytes_per_row % 4)) % 4;


    /*
     * Move both files to pixel data.
     */
    if (fseek(
            file1,
            header1.pixel_data_offset,
            SEEK_SET) != 0)
    {
        printf("Error: Could not locate pixel data.\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    if (fseek(
            file2,
            header2.pixel_data_offset,
            SEEK_SET) != 0)
    {
        printf("Error: Could not locate pixel data.\n");

        fclose(file1);
        fclose(file2);

        return 1;
    }


    /*
     * Statistics.
     */
    long total_bytes = 0;

    long changed_bytes = 0;

    long lsb_changes = 0;

    long total_difference = 0;

    int maximum_difference = 0;


    /*
     * Compare row by row.
     */
    for (int row = 0;
         row < info1.height;
         row++)
    {
        for (int column = 0;
             column < bytes_per_row;
             column++)
        {
            unsigned char byte1;
            unsigned char byte2;


            if (fread(&byte1, 1, 1, file1) != 1 ||
                fread(&byte2, 1, 1, file2) != 1)
            {
                printf(
                    "Error: Could not read pixel data.\n"
                );

                fclose(file1);
                fclose(file2);

                return 1;
            }


            total_bytes++;


            /*
             * Detect byte-level difference.
             */
            if (byte1 != byte2)
            {
                changed_bytes++;


                int difference =
                    abs(
                        (int)byte1 -
                        (int)byte2
                    );


                total_difference += difference;


                if (difference > maximum_difference)
                {
                    maximum_difference = difference;
                }


                /*
                 * Check whether the LSB changed.
                 */
                if ((byte1 & 1) !=
                    (byte2 & 1))
                {
                    lsb_changes++;
                }
            }
        }


        /*
         * Skip BMP row padding.
         *
         * Padding is not pixel data.
         */
        for (int p = 0;
             p < padding;
             p++)
        {
            unsigned char padding_byte1;
            unsigned char padding_byte2;

            fread(
                &padding_byte1,
                1,
                1,
                file1
            );

            fread(
                &padding_byte2,
                1,
                1,
                file2
            );
        }
    }


    /*
     * Calculate statistics.
     */
    double changed_percentage = 0.0;

    double lsb_change_percentage = 0.0;

    double average_difference = 0.0;


    if (total_bytes > 0)
    {
        changed_percentage =
            ((double)changed_bytes /
             (double)total_bytes) * 100.0;
    }


    if (total_bytes > 0)
    {
        lsb_change_percentage =
            ((double)lsb_changes /
             (double)total_bytes) * 100.0;
    }


    if (changed_bytes > 0)
    {
        average_difference =
            (double)total_difference /
            (double)changed_bytes;
    }


    /*
     * Display report.
     */
    printf("\n");
    printf("========================================\n");
    printf("       PIXELVAULT - IMAGE COMPARE\n");
    printf("========================================\n\n");


    printf("Image A : %s\n", image1);
    printf("Image B : %s\n", image2);


    /*
     * Image compatibility.
     */
    printf("\nImage Compatibility\n");
    printf("----------------------------------------\n");

    printf(
        "Dimensions        : MATCH (%d x %d)\n",
        info1.width,
        info1.height
    );

    printf(
        "Color Depth       : MATCH (24-bit)\n"
    );

    printf(
        "Compression       : NONE\n"
    );


    /*
     * Pixel comparison.
     */
    printf("\nPixel Comparison\n");
    printf("----------------------------------------\n");

    printf(
        "Bytes Compared    : %ld\n",
        total_bytes
    );

    printf(
        "Changed Bytes     : %ld\n",
        changed_bytes
    );

    printf(
        "Changed Percentage: %.4f%%\n",
        changed_percentage
    );

    printf(
        "Maximum Difference: %d\n",
        maximum_difference
    );

    printf(
        "Average Difference: %.4f\n",
        average_difference
    );


    /*
     * LSB comparison.
     */
    printf("\nLSB Analysis\n");
    printf("----------------------------------------\n");

    printf(
        "LSB Changes       : %ld\n",
        lsb_changes
    );

    printf(
        "LSB Change Rate   : %.4f%%\n",
        lsb_change_percentage
    );


    /*
     * Assessment.
     */
    printf("\nAssessment\n");
    printf("----------------------------------------\n");


    if (changed_bytes == 0)
    {
        printf(
            "No pixel-level modifications detected.\n"
        );
    }
    else if (
        lsb_changes == changed_bytes &&
        maximum_difference <= 1
    )
    {
        printf(
            "All detected changes affect the LSB "
            "and have a maximum difference of 1.\n"
        );

        printf(
            "This pattern is consistent with "
            "LSB-level modification.\n"
        );
    }
    else
    {
        printf(
            "Pixel-level modifications detected.\n"
        );

        printf(
            "Changes are not limited to simple "
            "LSB modifications.\n"
        );
    }


    printf(
        "\nNote: Comparison identifies differences "
        "but does not prove hidden data exists.\n"
    );


    printf("\n========================================\n");


    fclose(file1);
    fclose(file2);

    return 0;
}