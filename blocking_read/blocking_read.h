#ifdef _BLOCKING_READ_H_
#define _BLOCKING_READ_H_
#include <cstddef>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"

template<int D>
struct ap_axis_dk {
	ap_uint<D> data;
	ap_uint<1> last;
	ap_uint<D/8> keep;
};
template<int D>
struct ap_axis_dkt {
	ap_uint<D> data;
	ap_uint<1> last;
	ap_uint<D/8> keep;
	ap_uint<1> id;
};
#endif
