#include "blocking_read.h"

void blocking_read(hls::stream<ap_uint<8> > &in1,
		   hls::stream<ap_uint<8> > &in2,
		   hls::stream<ap_uint<8> > &outp) 
{
	ap_uint<8> in_1, in_2;
	in_1 = in1.read();
	if (in_1 == 'w') {
		printf("going into blocking read\n");
		in_2 = in2.read();
		outp.write(in_2);
	} else {
		outp.write(in_1);
	}
}
