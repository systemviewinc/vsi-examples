#include <cardano.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct rgb_pixel {
    char red;
    char green;
    char blue;
};

void compute(
    input_stream_int32 *data,  // Stream from platform/simulation
    output_stream_int32 *outstream // Output stream to platform/simulation
) {
    // 4 pixel is 3 ints
    rgb_pixel *tri_pixel;
    int32 pixelbuf[3];
    tri_pixel = (rgb_pixel *) &pixelbuf;
    for (int i = 0; i < sizeof(pixelbuf)/sizeof(int32); i++) {
        pixelbuf[i]= readincr(data);
    }

    for (int i = 0; i < sizeof(pixelbuf)/sizeof(rgb_pixel); i++) {
        // Keep only max color
        if (tri_pixel[i].red > tri_pixel[i].green && tri_pixel[i].red > tri_pixel[i].blue) {
            tri_pixel[i].green =0;
            tri_pixel[i].blue = 0;
        } else if (tri_pixel[i].green > tri_pixel[i].red && tri_pixel[i].green > tri_pixel[i].blue) {
            tri_pixel[i].red =0;
            tri_pixel[i].blue = 0;
        } else {
            tri_pixel[i].red = 0;
            tri_pixel[i].green =0;
        }
    }

    // Stream img out
    for (int i = 0; i < sizeof(pixelbuf)/sizeof(int32); i++) {
        writeincr(outstream, pixelbuf[i]);
    }
};
