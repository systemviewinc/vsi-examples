#ifndef SORT_X86_H
#define SORT_X86_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include <string.h>
#include <vsi_device.h>
#ifndef __VSI_HLS_SYN__
#include <unistd.h>
#endif

#define MAXLEN 1024
#define MAXSIZE 1024
#define DATA_WIDTH 32

template<int D>
struct ap_axis_dk {
	ap_uint<D> data;
	ap_uint<1> last;
	ap_uint<D/8> keep;
};

template<int D>
struct ap_axis_d {
	ap_uint<D> data;
	ap_uint<1> last;
};


#endif
