/**
 * Copyright (c) 2018 Scortex SAS
 */

#include "convolution.h"
#include <unistd.h>

template<typename axis_type,typename data_type, typename coef_type, int K>
static void convolution(int width, int height,
        hls::stream<axis_type> &src, hls::stream<axis_type> &dst,
        const coef_type coeff[K][K])
{
#pragma HLS DATAFLOW

    const int border_width = int(K / 2);
    hls::Window<K,1024,data_type> linebuf;
    #pragma HLS ARRAY_PARTITION variable=coeff dim=0 complete
    //data_type linebuf[K][1024];
    const int vconv_xlim = width - K +1;
    const int vconv_ylim = height + (K - 1);
//#pragma HLS INLINE // Into a DATAFLOW region
    ap_uint<K-1> col_sel = 0;
    // These assertions let HLS know the upper bounds of loops
    assert(height < MAX_IMG_ROWS);
    assert(width < MAX_IMG_COLS);
    assert(vconv_xlim < MAX_IMG_COLS - (K - 1));
    data_type in_val;
    axis_type in_axis;
    HLineBufH:for(int col = 0; col < vconv_ylim; col++) {
        HLineBufW:for(int row = 0; row < width; row++) {
        	#pragma HLS PIPELINE II=1
            if (col >= height)
            {
                in_val = 0;
            }
            else if (col == 0 && row == 0)
            {
            	in_axis = src.read();
            	in_val = in_axis.data;
            }
            else
            {
                in_val = src.read().data;
            }
            data_type tmp_buf0 = linebuf.getval(0, width-1);
            data_type tmp_buf1 = linebuf.getval(1, width-1);
            linebuf.shift_pixels_right();
            linebuf.insert_pixel(tmp_buf0,1,0);
            linebuf.insert_pixel(tmp_buf1,2,0);
            linebuf.insert_pixel(in_val,0,0);
            ap_int<26> out_val = 0;
            Conv:for(int i = 0; i < K; i++) {
                MultAdd:for(int j = 0; j < K; j++) {
                #pragma HLS unroll
                    out_val += linebuf.getval(i,width-K+j) * coeff[K-1-i][K-1-j];
                }
            }
            if ((col == K - 2 && row >= vconv_xlim) || (col > K - 2 && col < vconv_ylim -1) || (col == vconv_ylim - 1 && row < vconv_xlim))
            {
                axis_type axis_out;
                if (out_val > 255)
                {
                    out_val = 255;
                }
                else if (out_val < 0)
                {
                    out_val = 0;
                }
                axis_out.data = (data_type) out_val;
                if (row == vconv_xlim-1 && col > K - 2)
                {
                	axis_out.last = 1;
                }
                else
                {
                	axis_out.last = 0;
                }
                dst << axis_out;
            }
        }
        // Manage line buffer demultiplexing
        col_sel += 1;
        if (col_sel >= K)
        {
            col_sel = 0;
        }
    }
/*
    //Handle border by replicating the exact same pixels as orig, but in
    // a single loop taking the minimum (height*width) number of cycles
    Border:for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
        	data_type pix_in, l_edge_pix, r_edge_pix, pix_out;
#pragma HLS PIPELINE
            if (i == 0 || (i > border_width && i < height - border_width)) {
                // read a pixel out of the input stream and cache it for
                // immediate use and later replication purposes
                if (j < width - (K - 1)) {
                    pix_in = vconv.read();
                    borderbuf[j] = pix_in;
                }
                if (j == 0) {
                    l_edge_pix = pix_in;
                }
                if (j == width - K) {
                    r_edge_pix = pix_in;
                }
            }
            // Select output value from the appropriate cache resource
            if (j <= border_width) {
                pix_out = l_edge_pix;
            } else if (j >= width - border_width - 1) {
                pix_out = r_edge_pix;
            } else {
                pix_out = borderbuf[j - border_width];
            }
            dst << pix_out;
        }
    }*/
}

void conv3x3_sobel(hls::stream<axis_uint8_type> &input, hls::stream<axis_uint8_type> &output)
{
/*#pragma HLS INTERFACE axis port=input 
#pragma HLS INTERFACE axis port=output
#pragma HLS interface ap_ctrl_none port=return 
*/
//#pragma HLS INLINE region // bring loops in sub-functions to this DATAFLOW region
 while(1) {
        if (!input.empty()) {  
    		/*const int8_t filt_unit_coeff[3][3] = {
        	{0, 0, 0},
        	{0, 1, 0},
        	{0, 0, 0}};*/
    		const int8_t filt_unit_coeff[3][3] = {
        	{1, 0, -1},
        	{2, 0, -2},
        	{1, 0, -1}};
    		int width = 16;
    		int height = 16;
    		convolution<axis_uint8_type,uint8_type,int8_t,3>(width,height,input,output,filt_unit_coeff);
	}
	else {
		usleep(2);
	}
   }
}

void conv3x3_custom(hls::stream<axis_uint8_type> *input, hls::stream<axis_uint8_type> *output, int8_t coeff[3][3])
{
#pragma HLS INTERFACE s_axilite register port=coeff
#pragma HLS INTERFACE axis port=input 
#pragma HLS INTERFACE axis port=output
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INLINE region // bring loops in sub-functions to this DATAFLOW region
    int width = 10;
    int height = 10;
    convolution<axis_uint8_type,uint8_type,int8_t,3>(width,height,*input,*output,coeff);
}


