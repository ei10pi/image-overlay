#include <stdlib.h>
#include <stdio.h>

#include "video.h"
#include "overlay.h"

void create_video_frame(yuv420_video_frame *video_frame, u32 video_width, u32 video_height)
{
    video_frame->width = video_width;
    video_frame->height = video_height;
    video_frame->size = (video_width * video_height * 3) / 2;
    
    video_frame->data = (u8 *) malloc(video_frame->size);
    
    if (video_frame->data == NULL)
    {
        error_exit("Out of memory\n");   
    }
}

void process_video_frames(FILE *input_video, FILE *output_video, yuv420_image *yuv_image, yuv420_video_frame *video_frame)
{   
    while (fread(video_frame->data, 1, video_frame->size, input_video) == video_frame->size)
    {
        overlay_yuv420_image(yuv_image, video_frame);
        
        if (fwrite(video_frame->data, 1, video_frame->size, output_video) != video_frame->size)
        {
            fclose(input_video);
            fclose(output_video);
            error_exit("Failed to write the video data");
        }
    }
}
