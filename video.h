#if !defined(VIDEO_H)

#include <stdio.h>

#include "shared.h"
#include "yuv.h"

struct yuv420_video_frame
{
    u32 width;
    u32 height;
    u32 size;
    u8* data;  
};

void create_video_frame(yuv420_video_frame *video_frame, u32 video_width, u32 video_height);
void process_video_frames(FILE *input_video, FILE *output_video, yuv420_image *yuv_image, yuv420_video_frame *video_frame);

#define VIDEO_H

#endif
