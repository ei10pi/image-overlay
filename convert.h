#if !defined(CONVERT_H)

#include <immintrin.h>
#include "shared.h"
#include "bitmap.h"
#include "yuv.h"

typedef __m128i m128i;

struct lane_v3
{
    m128i r;
    m128i g;
    m128i b;
};
    
void convert_rgb_to_yuv420(u8 *rgb_data, u8 *yuv_data, u32 width, u32 y_index, u32 u_index, u32 v_index,
                                  u32 start_row, u32 end_row);

void convert_rgb_to_yuv420_ssse3(u8 *rgb_data, u8 *yuv_data, u32 width, u32 y_index, u32 u_index, u32 v_index,
                                  u32 start_row, u32 end_row);

void convert_rgb_to_yuv420_parallel(bitmap_image *bmp_image, yuv420_image *yuv_image);

#define CONVERT_H

#endif
