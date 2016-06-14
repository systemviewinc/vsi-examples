

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;
#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include "ref_test.h"

void 
empty_test(
	   hls::stream<ap_axis_d<32> > &in_a ,
	   hls::stream<ap_axis_d<32> > &in_b , 
	   hls::stream<ap_axis_d<32> > &out_c)
{
	ap_axis_d<32> outs;
	if (!in_a.empty()) outs = in_a.read();
	else if (!in_b.empty()) outs = in_b.read();
	out_c.write(outs);
}
void ref_test(int &in_int, int &out_int)
{
	for (int i = 0 ; i < 10; i++)
		out_int = in_int ;
}

