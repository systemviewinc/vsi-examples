#include "stream_mux.h"

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

void create_stream(hls::stream<ap_uint<DATA_WIDTH> >   &ind,
		   hls::stream<ap_axis_dk<DATA_WIDTH> >&outs) 
{
	// packets of 16 ap_uint<DATA_WIDTH>
	for (int i = 0 ; i < 8 ; i++) {
		ap_axis_dk<DATA_WIDTH> out;
		out.data = ind.read();
		out.keep = (i == 7) ? 3 : -1;
		out.last = (i == 7);
		outs.write(out);
	}
}

void strip_stream(hls::stream<ap_axis_dk<DATA_WIDTH> > &ins,
		  hls::stream<ap_uint<DATA_WIDTH> >   &outd)
{
	while (!ins.empty()) {
		outd.write(ins.read().data);
	}
}
