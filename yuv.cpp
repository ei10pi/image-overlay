#include <stdlib.h>

#include "yuv.h"

void create_yuv420_image(yuv420_image *yuv_image, bitmap_image *bmp_image)
{
    yuv_image->width = bmp_image->header.width;
    yuv_image->height = bmp_image->header.height;
    
    u32 yuv_image_size = (yuv_image->width * yuv_image->height * 3) / 2;
    yuv_image->data = (u8 *)malloc(yuv_image_size);

    if (yuv_image->data == NULL)
    {
        error_exit("Out of memory\n");   
    }
}
