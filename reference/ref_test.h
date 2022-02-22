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
template<int D> 
struct ap_axis_d {
	ap_uint<D> data;
	ap_uint<1> last;
};
