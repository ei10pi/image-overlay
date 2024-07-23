#include <stdio.h>
#include <stdlib.h>

#include "shared.h"
#include "bitmap.h"
#include "yuv.h"
#include "convert.h"
#include "video.h"

int main(int argc, char *argv[])
{    
    if (argc != 6)
    {
        fprintf(stderr, "\nUsage: %s -i -v -r -w -h\n\n", argv[0]);
        printf("-i: input image name\n");
        printf("-v: input video name\n");
        printf("-r: output video name\n");
        printf("-w: video width\n");
        printf("-h: video height\n");

        return 1;
    }
    
    const char *input_image_name = argv[1];
    const char *input_video_name = argv[2];
    const char *output_video_name = argv[3];
    u32 video_width = atoi(argv[4]);
    u32 video_height = atoi(argv[5]);

    bitmap_image bmp_image;
    load_bitmap_image(input_image_name, &bmp_image, video_width, video_height);     

    yuv420_image yuv_image;
    create_yuv420_image(&yuv_image, &bmp_image);    
    convert_rgb_to_yuv420_parallel(&bmp_image, &yuv_image);
    
    free(bmp_image.rgb_data);

    yuv420_video_frame video_frame;
    create_video_frame(&video_frame, video_width, video_height);

    FILE *input_video = fopen(input_video_name, "rb");

    if (!input_video)
    {
        error_exit("Unable to open the input video file\n");
    }

    FILE *output_video = fopen(output_video_name, "wb");

    if (!output_video)
    {
        error_exit("Unable to open the output video file\n");
    }
    
    process_video_frames(input_video, output_video, &yuv_image, &video_frame);

    fclose(input_video);
    fclose(output_video);
    
    free(yuv_image.data);
    free(video_frame.data);
        
    return 0;
}
