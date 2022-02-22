#ifndef REGRESSION_PROC_H
#define REGRESSION_PROC_H


#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string.h>
using namespace std;
#include <hls_stream.h>
//#include <ap_utils.h>
//#include "ap_axi_sdata.h"
#include <ap_int.h>
#include <vsi_device.h>


#define DATA_WIDTH 32
#define HDMI_WIDTH 24
#define DMA_WIDTH 512
#define K_WIDTH 256
#define K_BYTES K_WIDTH/8



template<int D>
struct ap_axis_dk {
	ap_uint<D> data;
	ap_uint<1> last;
};

typedef ap_uint<512> uint512_t;

#endif //REGRESSION_PROC_H
