#ifndef MICROPHONE_H
#define MICROPHONE_H
#include <unistd.h>
#include <hls_stream.h>

#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include <limits.h>
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/plot.hpp"
#include <vector>
#endif
template<int D>
struct ap_axis_md {
	ap_uint<D> 	data;
	ap_uint<1> 	last;
};

typedef struct _fft_data {
	float re;	// real part
	float im;	// imaginary part
	static const int width = 2*sizeof(float)*8; // width in bits
} fft_data;

struct fft_data_s {
	fft_data	data;	
	ap_uint<1>	last;	// end of packet
};

struct fft_amp {
	ap_uint<32> 	data;
	ap_uint<1> 	last;
};

#define SAMPLES 	512
#define QSPI_FIFO_SIZE 	16
#endif
