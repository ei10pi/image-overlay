#if !defined(BITMAP_H)

#include "shared.h"

#pragma pack(push, 1)
struct bitmap_header
{
    u16 file_type; 
    u32 file_size;
    u16 reserved1;
    u16 reserved2;
    u32 bitmap_offset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 size_of_bitmap;
    s32 horz_resolution;
    s32 vert_resolution;
    u32 colors_used;
    u32 colors_important;
};
#pragma pack(pop)

struct bitmap_image
{
    bitmap_header header;
    u8 *rgb_data;
};

void load_bitmap_image(const char *filename, bitmap_image *bmp_image, u32 video_width, u32 video_height);

#define BITMAP_H

#endif
