#if !defined(YUV_H)

#include "shared.h"
#include "bitmap.h"

struct yuv420_image
{
    u32 width;
    u32 height;
    u8 *data;
};

void create_yuv420_image(yuv420_image *yuv_image, bitmap_image *bmp_image);

#define YUV_H

#endif
