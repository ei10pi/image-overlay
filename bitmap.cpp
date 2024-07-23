#include <stdlib.h>
#include <stdio.h>
#include "bitmap.h"

void load_bitmap_image(const char *filename, bitmap_image *bmp_image, u32 video_width, u32 video_height)
{
    FILE *input_image;
    input_image = fopen(filename, "rb");
    if (!input_image)
    {
        error_exit("Unable to open the image file\n");
    }
    
    fread(&bmp_image->header, sizeof(bitmap_header), 1, input_image);
        
    if (bmp_image->header.file_type != 0x4D42)
    {
        error_exit("The image file is not a BMP file\n");      
    }
    else if (bmp_image->header.compression != 0)
    {
        error_exit("The BMP file should be uncompressed\n");
    }
            
    if (bmp_image->header.width % 2 != 0)
    {
        error_exit("The image width should be an even number\n");
    }
    else if (bmp_image->header.height % 2 != 0)
    {
        error_exit("The image height should be an even number\n");
    } 

    if (bmp_image->header.width > video_width)
    {
        error_exit("The image width should not exceed the width of a video frame\n");
    }
    else if (bmp_image->header.height > video_height)
    {
        error_exit("The image height should not exceed the height of a video frame\n");
    }
    
    u32 rgb_data_size = bmp_image->header.height * bmp_image->header.width * 3;
    bmp_image->rgb_data = (u8 *)malloc(rgb_data_size);

    if (bmp_image->rgb_data == NULL)
    {
        error_exit("Out of memory\n");   
    }
                
    fseek(input_image, bmp_image->header.bitmap_offset, SEEK_SET);
    fread(bmp_image->rgb_data, rgb_data_size, 1, input_image);         
    fclose(input_image);            
}
