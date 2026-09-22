#include<stdio.h>
#include<string.h>
#include "bmp.h"

int main(int argc, char*argv[])
{
    if(argc<2)
    {
        printf("PixelVault\n");
        printf("A C-Based Image Stegnography & Steganalysis Toolkit\n\n");
        
        printf("Usage:\n");
        printf(" pixelvault <command>\n\n");

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

    if(strcmp(argv[1],"info")==0)
    {
        if(argc<3)
        {
            printf("Usage: pixelvault info <image.bmp>\n");
            return 1;
        }
        
        return read_bmp_info(argv[2]);
    }
    
    else if(strcmp(argv[1],"hide")==0)
    {
        printf("HIDE command selected.\n");
    }
    else if(strcmp(argv[1],"extract")==0)
    {
        printf("EXTRACT command selected.\n");
    }
    else if(strcmp(argv[1],"capacity")==0)
    {
        printf("CAPACITY command selected.\n");
    }
    else if(strcmp(argv[1],"compare")==0)
    {
        printf("COMPARE command selected.\n");
    }
    else if(strcmp(argv[1],"analyze")==0)
    {
        printf("ANALYZE command selected.\n");
    }
    else if(strcmp(argv[1],"detect")==0)
    {
        printf("DETECT command selected.\n");
    }
    else
    {
        printf("Unknown command: %s\n",argv[1]);
        return 1;
    }
    return 0;
}