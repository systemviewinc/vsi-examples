#ifndef ADC_SPI_H
#define ADC_SPI_H
#include <unistd.h>
#include <hls_stream.h>

#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include <limits.h>
#include <vsi_device.h>
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
