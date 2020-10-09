
#ifndef _IMGPROCESSING_H_
#define _IMGPROCESSING_H_

#define INPUT_IMAGE_PATH "lena512color.tiff"
#define HEIGHT 512
#define WIDTH 512

// 8bit RGB pixel
struct rgb_pixel {
    char red;
    char green;
    char blue;
};

// tiff format
struct img_struct {
    char head[8];
    rgb_pixel raw_img[HEIGHT*WIDTH];
    char tail[132];
};

#endif //_IMGPROCESSING_H_