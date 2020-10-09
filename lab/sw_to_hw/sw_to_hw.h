#ifndef SW_TO_HW_H
#define SW_TO_HW_H
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include <string.h>
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include <unistd.h>
#endif

#define DATA_WIDTH 32

template<int D>
struct ap_axis_d {
	ap_uint<D> data;
	ap_uint<1> last;
};
#endif
