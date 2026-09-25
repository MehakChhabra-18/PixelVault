#include<stdio.h>
#include<stdlib.h>

#include "bmp.h"
#include "compare.h"

int compare_images(
    const char *image1,
    const char *image2
)
{
    FILE *file1=fopen(image1,"rb");
    FILE *file2=fopen(image2,"rb");
    if(file1==NULL)
    {
        printf("Error: Could not open %s\n",image1);
        return 1;
    }

    if(file2==NULL)
    {
        printf("Error: Could not open %s\n",image2);
        fclose(file1);
        return 1;
    }

    BMPFileHeader header1;
    BMPFileHeader header2;
    BMPInfoHeader info1;
    BMPInfoHeader info2;

    fread(&header1,sizeof(BMPFileHeader),1,file1);
    fread(&info1,sizeof(BMPInfoHeader),1,file1);

    fread(&header2,sizeof(BMPFileHeader),1,file2);
    fread(&info2,sizeof(BMPInfoHeader),1,file2);

    if(header1.signature!=0x4D42 || header2.signature!=0x4D42)
    {
        printf("Error: One or both files are not valid BMP images.\n");
        fclose(file1);
        fclose(file2);
        return 1;
    }

    if(info1.width!=info2.width || info1.height!=info2.height)
    {
        printf("Error: Images have different dimensions.\n");
        printf("Image 1: %d X %d\n",info1.width,info1.height);
        printf("Image 2: %d X %d\n",info2.width,info2.height);
        fclose(file1);
        fclose(file2);
        return 1;
    }

    fseek(file1,header1.pixel_data_offset,SEEK_SET);
    fseek(file2,header2.pixel_data_offset,SEEK_SET);

    long total_pixels=(long)info1.width*(long)info1.height;
    long total_bytes=total_pixels*3;
    long different_bytes=0;
    long lsb_changes=0;
    int maximum_difference=0;

    for(long i=0;i<total_bytes;i++)
    {
        unsigned char byte1;
        unsigned char byte2;
        if(fread(&byte1,1,1,file1)!=1 || fread(&byte2,1,1,file2)!=1)
        {
            printf("Error: Could not read pixel data.\n");
            fclose(file1);
            fclose(file2);
            return 1;
        }

        if(byte1!=byte2)
        {
            different_bytes++;
            int difference=abs((int)byte1-(int)byte2);
            if(difference>maximum_difference)
            {
                maximum_difference=difference;
            }

            if((byte1&1)!=(byte2&1))
            {
                lsb_changes++;
            }
        }
    }

    double changed_percentage=((double)different_bytes/(double)total_bytes)*100.0;
    printf("\n");
    printf("======================================\n");
    printf("     PIXELVAULT - IMAGE COMPARE\n");
    printf("======================================\n");

    printf("Image A : %s\n",image1);
    printf("Image B : %s\n",image2);
    printf("\nDimensions\n");
    printf("--------------------------\n");
    printf("Width  : %d pixels\n",info1.width);
    printf("Height : %d pixels\n",info1.height);

    printf("\nPixel Analysis\n");
    printf("----------------------------\n");
    printf("Pixel bytes compared : %ld\n",total_bytes);
    printf("Different bytes      : %ld\n",different_bytes);
    printf("Changed percentage   : %.4f%%\n",changed_percentage);
    printf("LSB changes          : %ld\n",lsb_changes);
    printf("Maximum difference   : %d\n",maximum_difference);

    printf("\nResult\n");
    printf("-------------------\n");
    if(different_bytes==0)
    {
        printf("No pixel-level differences detected.\n");
    }
    else
    {
        printf("Pixel-level modifications detected.\n");
    }

    printf("=======================================\n");
    fclose(file1);
    fclose(file2);
    return 0;
}