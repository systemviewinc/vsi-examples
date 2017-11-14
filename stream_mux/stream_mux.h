#ifndef STREAM_MUX_H
#define STREAM_MUX_H
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include <unistd.h>
#endif

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
template<int D>
struct ap_axis_dkt {
	ap_uint<D> data;
	ap_uint<1> last;
	ap_uint<D/8> keep;
	ap_uint<1> id;
};

template<typename T,int packet_size=0> void stream_split(hls::stream<T> &ins,
							 hls::stream<T> &o1,
							 hls::stream<T> &o2)
{
	T idx;
#ifndef __VSI_HLS_SYN__
	if (packet_size != 0) {
		do {
			unsigned char _lbuff[packet_size];
			int rv =0 ;
			unsigned int szr = 0;
			do {
				rv = ins.read(&_lbuff[szr],packet_size - szr);
				assert(rv >= 0);
				szr += rv;
			} while (szr < packet_size && rv > 0);
			o1.write(_lbuff,packet_size);
			o2.write(_lbuff,packet_size);
		} while (1);
	}
#endif
	do {
#pragma HLS PIPELINE II=1
		idx = ins.read();
		o1.write(idx);
		o2.write(idx);
	} while (1);
}

#endif
