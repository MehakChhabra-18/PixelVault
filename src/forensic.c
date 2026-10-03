#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "forensic.h"


int forensic_analysis(
    const char *original_filename,
    const char *suspect_filename
)
{
    FILE *original =
        fopen(original_filename, "rb");

    FILE *suspect =
        fopen(suspect_filename, "rb");


    if (original == NULL)
    {
        printf(
            "Error: Could not open original image.\n"
        );

        return 1;
    }


    if (suspect == NULL)
    {
        printf(
            "Error: Could not open suspect image.\n"
        );

        fclose(original);

        return 1;
    }


    /*
     * Read BMP headers.
     */
    BMPFileHeader original_file_header;
    BMPFileHeader suspect_file_header;

    BMPInfoHeader original_info;
    BMPInfoHeader suspect_info;


    if (fread(
            &original_file_header,
            sizeof(BMPFileHeader),
            1,
            original) != 1)
    {
        printf(
            "Error: Could not read original BMP header.\n"
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    if (fread(
            &original_info,
            sizeof(BMPInfoHeader),
            1,
            original) != 1)
    {
        printf(
            "Error: Could not read original BMP information.\n"
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    if (fread(
            &suspect_file_header,
            sizeof(BMPFileHeader),
            1,
            suspect) != 1)
    {
        printf(
            "Error: Could not read suspect BMP header.\n"
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    if (fread(
            &suspect_info,
            sizeof(BMPInfoHeader),
            1,
            suspect) != 1)
    {
        printf(
            "Error: Could not read suspect BMP information.\n"
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    /*
     * Validate BMP files.
     */
    if (original_file_header.signature != 0x4D42 ||
        suspect_file_header.signature != 0x4D42)
    {
        printf(
            "Error: One or both files are not valid BMP images.\n"
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    /*
     * PixelVault supports uncompressed 24-bit BMP.
     */
    if (original_info.bits_per_pixel != 24 ||
        suspect_info.bits_per_pixel != 24 ||
        original_info.compression != 0 ||
        suspect_info.compression != 0)
    {
        printf(
            "Error: Only uncompressed 24-bit BMP images "
            "are supported.\n"
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    /*
     * Images must have the same dimensions.
     */
    if (original_info.width != suspect_info.width ||
        original_info.height != suspect_info.height)
    {
        printf(
            "Error: Images have different dimensions.\n"
        );

        printf(
            "Original: %d x %d\n",
            original_info.width,
            original_info.height
        );

        printf(
            "Suspect : %d x %d\n",
            suspect_info.width,
            suspect_info.height
        );

        fclose(original);
        fclose(suspect);

        return 1;
    }


    /*
     * Calculate pixel row information.
     *
     * 24-bit BMP = 3 bytes per pixel.
     */
    int bytes_per_row =
        original_info.width * 3;

    int padding =
        (4 - (bytes_per_row % 4)) % 4;


    /*
     * Move both files to pixel data.
     */
    fseek(
        original,
        original_file_header.pixel_data_offset,
        SEEK_SET
    );

    fseek(
        suspect,
        suspect_file_header.pixel_data_offset,
        SEEK_SET
    );


    /*
     * Statistics.
     */
    long total_bytes = 0;

    long changed_bytes = 0;

    long lsb_changes = 0;

    long total_difference = 0;

    int maximum_difference = 0;


    /*
     * Compare pixel data.
     */
    for (int row = 0;
         row < original_info.height;
         row++)
    {
        for (int column = 0;
             column < bytes_per_row;
             column++)
        {
            unsigned char original_byte;
            unsigned char suspect_byte;


            if (fread(
                    &original_byte,
                    1,
                    1,
                    original) != 1 ||
                fread(
                    &suspect_byte,
                    1,
                    1,
                    suspect) != 1)
            {
                printf(
                    "Error: Could not read pixel data.\n"
                );

                fclose(original);
                fclose(suspect);

                return 1;
            }


            total_bytes++;


            if (original_byte != suspect_byte)
            {
                changed_bytes++;


                int difference =
                    abs(
                        (int)original_byte -
                        (int)suspect_byte
                    );


                total_difference += difference;


                if (difference > maximum_difference)
                {
                    maximum_difference = difference;
                }


                /*
                 * Check whether the LSB changed.
                 */
                if ((original_byte & 1) !=
                    (suspect_byte & 1))
                {
                    lsb_changes++;
                }
            }
        }


        /*
         * Skip BMP padding.
         */
        for (int p = 0;
             p < padding;
             p++)
        {
            unsigned char padding1;
            unsigned char padding2;

            fread(
                &padding1,
                1,
                1,
                original
            );

            fread(
                &padding2,
                1,
                1,
                suspect
            );
        }
    }


    fclose(original);
    fclose(suspect);


    /*
     * Calculate percentages.
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
     * Calculate how many changed bytes
     * are also LSB changes.
     */
    double lsb_change_ratio = 0.0;


    if (changed_bytes > 0)
    {
        lsb_change_ratio =
            ((double)lsb_changes /
             (double)changed_bytes) * 100.0;
    }


    /*
     * Display forensic report.
     */
    printf("\n");
    printf("========================================\n");
    printf("      PIXELVAULT - FORENSIC REPORT\n");
    printf("========================================\n\n");


    printf("Original Image : %s\n",
           original_filename);

    printf("Suspect Image  : %s\n\n",
           suspect_filename);


    /*
     * Image information.
     */
    printf("Image Compatibility\n");
    printf("----------------------------------------\n");

    printf(
        "Dimensions     : MATCH (%d x %d)\n",
        original_info.width,
        original_info.height
    );

    printf(
        "Color Depth    : MATCH (24-bit)\n"
    );

    printf(
        "Compression    : NONE\n"
    );


    /*
     * Pixel changes.
     */
    printf("\nPixel Changes\n");
    printf("----------------------------------------\n");

    printf(
        "Bytes Compared      : %ld\n",
        total_bytes
    );

    printf(
        "Changed Bytes       : %ld\n",
        changed_bytes
    );

    printf(
        "Changed Percentage  : %.4f%%\n",
        changed_percentage
    );

    printf(
        "Maximum Difference  : %d\n",
        maximum_difference
    );

    printf(
        "Average Difference  : %.4f\n",
        average_difference
    );


    /*
     * LSB analysis.
     */
    printf("\nLSB Analysis\n");
    printf("----------------------------------------\n");

    printf(
        "LSB Changes         : %ld\n",
        lsb_changes
    );

    printf(
        "LSB Change Rate     : %.4f%%\n",
        lsb_change_percentage
    );

    printf(
        "Changed Bytes that affect LSB : %.2f%%\n",
        lsb_change_ratio
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

        printf(
            "The two images appear identical at "
            "the pixel-byte level.\n"
        );
    }
    else if (
        lsb_changes == changed_bytes &&
        maximum_difference <= 1
    )
    {
        printf(
            "Pixel modifications were detected.\n\n"
        );

        printf(
            "All detected changes affected the LSB.\n"
        );

        printf(
            "Maximum modification magnitude is 1.\n"
        );

        printf(
            "This pattern is consistent with "
            "LSB-level modification.\n"
        );
    }
    else
    {
        printf(
            "Pixel modifications were detected.\n\n"
        );

        printf(
            "The changes are not limited to simple "
            "LSB modifications.\n"
        );
    }


    printf(
        "\nImportant:\n"
    );

    printf(
        "This report identifies pixel-level evidence.\n"
    );

    printf(
        "It does NOT prove that hidden data exists.\n"
    );


    printf("\n========================================\n");


    return 0;
}