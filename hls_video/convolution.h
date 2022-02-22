/**

 * Copyright (c) 2018 Scortex SAS

 */



#ifndef SRC_CONV_3x3_H

#define SRC_CONV_3x3_H



#include <hls_stream.h>

#include <ap_int.h>

#include <hls_video.h>





//#include "axi_stream_config.h"

#include <stdint.h>

#include <assert.h>



template<int D,int U,int TI,int TD>

struct hls_ap_axiu{

     ap_uint<D>   data;

     ap_uint<1>   last;

};



typedef hls_ap_axiu<8,1,1,1> axis_uint8_type;

typedef hls::stream<axis_uint8_type> stream_uint8_type;

typedef uint32_t uint32_type;

typedef uint16_t uint16_type;

typedef uint8_t uint8_type;



//typedef uint8_t coef_size;

//typedef uint32_t coef_size;



#define MAX_IMG_ROWS 1080

#define MAX_IMG_COLS 1920



void conv3x3_sobel(stream_uint8_type *input, stream_uint8_type *output);

void conv3x3_custom(stream_uint8_type *input, stream_uint8_type *output,int8_t coeff[3][3]);



#endif /* SRC_CONV_3x3_H */


