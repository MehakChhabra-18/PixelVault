#include<stdio.h>
#include "bmp.h"

int read_bmp_info(const char *filename)
{
    FILE *file=fopen(filename,"rb");
    if(file==NULL)
    {
        printf("Error: Could not open file %s\n",filename);
        return 1;
    }

    BMPFileHeader file_header;

    fread(&file_header.signature,sizeof(file_header.signature),1,file);
    fread(&file_header.file_size,sizeof(file_header.file_size),1,file);
    fread(&file_header.pixel_data_offset,sizeof(file_header.pixel_data_offset),1,file);

    printf("File size: %u\n",file_header.file_size);
    printf("Pixel data offset: %u\n",file_header.pixel_data_offset);
    fclose(file);
    return 0;
}