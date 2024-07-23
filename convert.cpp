#include <thread>
#include <vector>
#include <immintrin.h>

#include "convert.h"
#include "yuv.h"

static inline
u8 compute_y(u8 r, u8 g, u8 b)
{
    return (66 * r + 129 * g + 25 * b) >> 8;
}


static inline
u8 compute_u(u8 r, u8 g, u8 b)
{
    return ((-38 * r - 74 * g + 112 * b) >> 8) + 128;
}


static inline
u8 compute_v(u8 r, u8 g, u8 b)
{
    return ((112 * r - 94 * g - 18 * b) >> 8) + 128;
}

static inline
lane_v3 set1_epi16(s16 a, s16 b, s16 c)
{
    lane_v3 result;
    result.r = _mm_set1_epi16(c);
    result.g = _mm_set1_epi16(b);
    result.b = _mm_set1_epi16(a);

    return result;
}


static inline
lane_v3 set_epi8_lo(u8 a, u8 b, u8 c, u8 d)
{

    lane_v3 result;
    result.r = _mm_set_epi8(
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, d+2,  0xff, c+2,
        0xff, b+2,  0xff, a+2);
    result.g = _mm_set_epi8(
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, d+1,  0xff, c+1,
        0xff, b+1,  0xff, a+1);
    
    result.b = _mm_set_epi8(
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, d,    0xff, c,
        0xff, b,    0xff, a);
    
    return result;
}


static inline
lane_v3 set_epi8_hi(u8 a, u8 b, u8 c, u8 d)
{
    lane_v3 result;
    result.r = _mm_set_epi8(
        0xff, d+2,  0xff, c+2,
        0xff, b+2,  0xff, a+2,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff);

    result.g = _mm_set_epi8(
        0xff, d+1,  0xff, c+1,
        0xff, b+1,  0xff, a+1,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff);
    
    result.b = _mm_set_epi8(
        0xff, d,    0xff, c,
        0xff, b,    0xff, a,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff);

    return result;
}


void convert_rgb_to_yuv420(u8 *rgb_data, u8 *yuv_data, u32 width, u32 y_index, u32 u_index, u32 v_index,
                           u32 start_row, u32 end_row)
{
    u8 r, g, b;
    u32 pixel_row;

    for (u32 y = start_row; y > end_row; y -= 2)
    {
        pixel_row = (y - 1) * width;
        
        for (u32 x = 0; x < width; x += 2)
        {   
            r = rgb_data[((x + pixel_row) * 3) + 2];
            g = rgb_data[((x + pixel_row) * 3) + 1];
            b = rgb_data[(x + pixel_row) * 3];
                        
            yuv_data[y_index++] = compute_y(r, g, b);
            yuv_data[u_index++] = compute_u(r, g, b);
            yuv_data[v_index++] = compute_v(r, g, b);


            r = rgb_data[((x + 1 + pixel_row) * 3) + 2];
            g = rgb_data[((x + 1 + pixel_row) * 3) + 1];
            b = rgb_data[(x + 1 + pixel_row) * 3];
                
            yuv_data[y_index++] = compute_y(r, g, b);               
        }
        
        pixel_row -= width;
        
        for (u32 x = 0; x < width; x++)
        {   
            r = rgb_data[((x + pixel_row) * 3) + 2];
            g = rgb_data[((x + pixel_row) * 3) + 1];
            b = rgb_data[(x + pixel_row) * 3];
                        
            yuv_data[y_index++] = compute_y(r, g, b);
        }
    }
}


void convert_rgb_to_yuv420_ssse3(u8 *rgb_data, u8 *yuv_data, u32 width, u32 y_index, u32 u_index, u32 v_index,
                                 u32 start_row, u32 end_row)
{

    u8 r, g, b;
    u32 simd_aligned_width = width & (~15);
    
    u32 pixel_row_1, pixel_row_2;
    u32 x;

    lane_v3 rgb_shuffle_lo = set_epi8_lo(0, 3, 6, 9);
    lane_v3 rgb_shuffle_hi = set_epi8_hi(0, 3, 6, 9);
    
    m128i y_shuffle_lo = _mm_set_epi8(
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        15,   13,   11,   9,
        7,    5,    3,    1);
    m128i y_shuffle_hi = _mm_set_epi8(
        15,   13,   11,   9,
        7,    5,    3,    1,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff);

    m128i uv_shuffle_lo_1 = _mm_set_epi8(
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        12,   8,    4,    0);
    m128i uv_shuffle_lo_2 = _mm_set_epi8(
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        12,   8,    4,    0,
        0xff, 0xff, 0xff, 0xff);
    
    lane_v3 y_coeff = set1_epi16(25, 129, 66);
    lane_v3 u_coeff = set1_epi16(112, -74, -38);
    lane_v3 v_coeff = set1_epi16(-18, -94, 112);

    m128i uv_const = _mm_set1_epi32(128);
     
    for (u32 row = start_row; row > end_row; row -= 2)
    {
        pixel_row_1 = (row - 1) * width;
        pixel_row_2 = (row - 2) * width;
        
        for (x = 0; x < simd_aligned_width; x += 16)
        {
            m128i rgb_lane_1 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_1) * 3]);
            m128i rgb_lane_2 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_1) * 3 + 12]);
            m128i rgb_lane_3 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_1) * 3 + 24]);
            m128i rgb_lane_4 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_1) * 3 + 36]);
           
            m128i rgb_lane_5 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_2) * 3]);
            m128i rgb_lane_6 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_2) * 3 + 12]);
            m128i rgb_lane_7 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_2) * 3 + 24]);
            m128i rgb_lane_8 = _mm_lddqu_si128((m128i* )&rgb_data[(x + pixel_row_2) * 3 + 36]);


            m128i red_lane_1 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_1, rgb_shuffle_lo.r), _mm_shuffle_epi8(rgb_lane_2, rgb_shuffle_hi.r));
            m128i green_lane_1 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_1, rgb_shuffle_lo.g), _mm_shuffle_epi8(rgb_lane_2, rgb_shuffle_hi.g));
            m128i blue_lane_1 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_1, rgb_shuffle_lo.b), _mm_shuffle_epi8(rgb_lane_2, rgb_shuffle_hi.b));
            
            m128i red_lane_2 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_3, rgb_shuffle_lo.r), _mm_shuffle_epi8(rgb_lane_4, rgb_shuffle_hi.r));
            m128i green_lane_2 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_3, rgb_shuffle_lo.g), _mm_shuffle_epi8(rgb_lane_4, rgb_shuffle_hi.g));
            m128i blue_lane_2 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_3, rgb_shuffle_lo.b), _mm_shuffle_epi8(rgb_lane_4, rgb_shuffle_hi.b));
            
            m128i red_lane_3 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_5, rgb_shuffle_lo.r), _mm_shuffle_epi8(rgb_lane_6, rgb_shuffle_hi.r));
            m128i green_lane_3 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_5, rgb_shuffle_lo.g), _mm_shuffle_epi8(rgb_lane_6, rgb_shuffle_hi.g));
            m128i blue_lane_3 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_5, rgb_shuffle_lo.b), _mm_shuffle_epi8(rgb_lane_6, rgb_shuffle_hi.b));
            
            m128i red_lane_4 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_7, rgb_shuffle_lo.r), _mm_shuffle_epi8(rgb_lane_8, rgb_shuffle_hi.r));
            m128i green_lane_4 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_7, rgb_shuffle_lo.g), _mm_shuffle_epi8(rgb_lane_8, rgb_shuffle_hi.g));
            m128i blue_lane_4 = _mm_or_si128(_mm_shuffle_epi8(rgb_lane_7, rgb_shuffle_lo.b), _mm_shuffle_epi8(rgb_lane_8, rgb_shuffle_hi.b));
            

            m128i y_lane_1 = _mm_add_epi16(_mm_mullo_epi16(red_lane_1, y_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_1, y_coeff.g), _mm_mullo_epi16(blue_lane_1, y_coeff.b)));
            m128i y_lane_2 = _mm_add_epi16(_mm_mullo_epi16(red_lane_2, y_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_2, y_coeff.g), _mm_mullo_epi16(blue_lane_2, y_coeff.b)));

            m128i u_lane_1 = _mm_add_epi16(_mm_srai_epi32(_mm_add_epi16(_mm_mullo_epi16(red_lane_1, u_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_1, u_coeff.g), _mm_mullo_epi16(blue_lane_1, u_coeff.b))), 8), uv_const);
            m128i u_lane_2 = _mm_add_epi16(_mm_srai_epi32(_mm_add_epi16(_mm_mullo_epi16(red_lane_2, u_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_2, u_coeff.g), _mm_mullo_epi16(blue_lane_2, u_coeff.b))), 8), uv_const);

            m128i v_lane_1 = _mm_add_epi16(_mm_srai_epi32(_mm_add_epi16(_mm_mullo_epi16(red_lane_1, v_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_1, v_coeff.g), _mm_mullo_epi16(blue_lane_1, v_coeff.b))), 8), uv_const);
            m128i v_lane_2 = _mm_add_epi16(_mm_srai_epi32(_mm_add_epi16(_mm_mullo_epi16(red_lane_2, v_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_2, v_coeff.g), _mm_mullo_epi16(blue_lane_2, v_coeff.b))), 8), uv_const);

            y_lane_1 = _mm_shuffle_epi8(y_lane_1, y_shuffle_lo);
            y_lane_2 = _mm_shuffle_epi8(y_lane_2, y_shuffle_hi);
            
            u_lane_1 = _mm_shuffle_epi8(u_lane_1, uv_shuffle_lo_1);
            u_lane_2 = _mm_shuffle_epi8(u_lane_2, uv_shuffle_lo_2);
            
            v_lane_1 = _mm_shuffle_epi8(v_lane_1, uv_shuffle_lo_1);
            v_lane_2 = _mm_shuffle_epi8(v_lane_2, uv_shuffle_lo_2);

            _mm_storeu_si128((m128i* )&yuv_data[y_index], _mm_or_si128(y_lane_1, y_lane_2));
            _mm_storel_epi64((m128i* )&yuv_data[u_index], _mm_or_si128(u_lane_1, u_lane_2));
            _mm_storel_epi64((m128i* )&yuv_data[v_index], _mm_or_si128(v_lane_1, v_lane_2));
         
                                                       
            m128i y_lane_3 = _mm_add_epi16(_mm_mullo_epi16(red_lane_3, y_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_3, y_coeff.g), _mm_mullo_epi16(blue_lane_3, y_coeff.b)));
            m128i y_lane_4 = _mm_add_epi16(_mm_mullo_epi16(red_lane_4, y_coeff.r), _mm_add_epi16(_mm_mullo_epi16(green_lane_4, y_coeff.g), _mm_mullo_epi16(blue_lane_4, y_coeff.b)));

            y_lane_3 = _mm_shuffle_epi8(y_lane_3, y_shuffle_lo);
            y_lane_4 = _mm_shuffle_epi8(y_lane_4, y_shuffle_hi);

            _mm_storeu_si128((m128i* )&yuv_data[y_index + width], _mm_or_si128(y_lane_3, y_lane_4));

            
            y_index += 16;
            u_index += 8;
            v_index += 8;
        }

        // handle leftovers 
        if (simd_aligned_width != width)
        {
            for (; x < width; x += 2)
            {   
                r = rgb_data[((x + pixel_row_1) * 3) + 2];
                g = rgb_data[((x + pixel_row_1) * 3) + 1];
                b = rgb_data[(x + pixel_row_1) * 3];
                        
                yuv_data[y_index++] = compute_y(r, g, b); 
                yuv_data[u_index++] = compute_u(r, g, b);
                yuv_data[v_index++] = compute_v(r, g, b);


                r = rgb_data[((x + 1 + pixel_row_1) * 3) + 2];
                g = rgb_data[((x + 1 + pixel_row_1) * 3) + 1];
                b = rgb_data[(x + 1 + pixel_row_1) * 3];
                        
                yuv_data[y_index++] = compute_y(r, g, b);
            }
        
            for (x = simd_aligned_width; x < width; x++)
            {   
                r = rgb_data[((x + pixel_row_2) * 3) + 2];
                g = rgb_data[((x + pixel_row_2) * 3) + 1];
                b = rgb_data[(x + pixel_row_2) * 3];
                        
                yuv_data[y_index + x] = compute_y(r, g, b);
            }
        }

        y_index += width;
    }

}

void convert_rgb_to_yuv420_parallel(bitmap_image *bmp_image, yuv420_image *yuv_image)
{
    u32 image_size = yuv_image->width * yuv_image->height;
    
    u32 y_index = 0;
    u32 u_index = image_size;
    u32 v_index = image_size + (image_size / 4);

    u32 thread_count = 4;

    u32 height_partition = (yuv_image->height / thread_count) & (~1);
    
    u32 start_row = yuv_image->height;
    u32 end_row = yuv_image->height - height_partition;

    std::vector<std::thread> threads(thread_count - 1);
    for (u32 i = 0; i < thread_count - 1; i++)
    {
        threads[i] = std::thread(convert_rgb_to_yuv420_ssse3, bmp_image->rgb_data, yuv_image->data, yuv_image->width, y_index,
                                 u_index, v_index, start_row, end_row);
        y_index += yuv_image->width * height_partition;
        u_index += (yuv_image->width * height_partition) / 4;
        v_index += (yuv_image->width * height_partition) / 4;
        start_row -= height_partition;
        end_row -= height_partition;
    }

    convert_rgb_to_yuv420_ssse3(bmp_image->rgb_data, yuv_image->data, yuv_image->width, y_index, u_index, v_index, start_row, 0);
        
    for (u32 i = 0; i < thread_count - 1; i++)
    {
        threads[i].join();
    }
}
