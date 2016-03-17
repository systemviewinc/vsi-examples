#include "stream_mux.h"

#define DATA_WIDTH 32
//#define ARB_ON_LAST

ap_axis_dkt<DATA_WIDTH> wout(hls::stream<ap_axis_dk<DATA_WIDTH> > &inp, ap_uint<1> tid) 
{
	ap_axis_dk<DATA_WIDTH> in = inp.read();
	ap_axis_dkt<DATA_WIDTH> out;
	out.data = in.data;
	out.last = in.last;
	out.keep = in.keep;
	out.id   = tid;
	return out;
}

void stream_mux (hls::stream<ap_axis_dk<DATA_WIDTH> > &in1,
		 hls::stream<ap_axis_dk<DATA_WIDTH> > &in2,
		 hls::stream<ap_axis_dkt<DATA_WIDTH> > &outp)
{
	while (!in1.empty() || !in2.empty()) {
#pragma HLS PIPELINE II=1
		ap_axis_dkt<DATA_WIDTH> out;
		ap_axis_dk<DATA_WIDTH> in;
		ap_uint<1> tid;
		bool set = false;
		if (!in1.empty()) {
			in = in1.read();
			tid = 0;
			set = true;
		} else if (!in2.empty()) {
			in = in2.read();
			tid = 1;
			set = true;
		}
		out.data = in.data;
		out.keep = in.keep;
		out.last = in.last;
		out.id   = tid;
		if (set) outp.write(out);
	} 
}
