#include "overlay.h"
#include "video.h"

void overlay_yuv420_image(yuv420_image *yuv_image, yuv420_video_frame *video_frame)
{
    
    u8 *video_y_ptr = video_frame->data;
    u8 *video_u_ptr = video_frame->data + video_frame->width * video_frame->height;
    u8 *video_v_ptr = video_u_ptr + (video_frame->width / 2) * (video_frame->height / 2);

    u8 *image_y_ptr = yuv_image->data;
    u8 *image_u_ptr = yuv_image->data + yuv_image->width * yuv_image->height;
    u8 *image_v_ptr = image_u_ptr + (yuv_image->width / 2) * (yuv_image->height / 2);
            
    for (u32 y = 0; y < yuv_image->height; y += 2)
    {
        u32 video_row = y * video_frame->width;
        u32 image_row = y * yuv_image->width;

        for (u32 x = 0; x < yuv_image->width; x += 2)
        {
            video_y_ptr[video_row + x] = image_y_ptr[image_row + x];

            int video_u_pos = video_row / 4 + x / 2;
            int video_v_pos = video_row / 4 + x / 2;
            int image_u_pos = image_row / 4 + x / 2;
            int image_v_pos = image_row / 4 + x / 2;

            video_u_ptr[video_u_pos] = image_u_ptr[image_u_pos];
            video_v_ptr[video_v_pos] = image_v_ptr[image_v_pos];

            video_y_ptr[video_row + x + 1] = image_y_ptr[image_row + x + 1];
                
        }

        video_row += video_frame->width;
        image_row += yuv_image->width;
        for (u32 x = 0; x < yuv_image->width; x++)
        {                               
            video_y_ptr[video_row + x] = image_y_ptr[image_row + x];
        }
    }

}
