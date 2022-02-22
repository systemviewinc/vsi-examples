#include <hls_stream.h>
#include "ap_axi_sdata.h"

typedef ap_axiu<24, 1, 1, 1> AXI_24_VALUE;

template<int D,int U,int TI,int TD>
  struct ap_axiu_type{
    ap_uint<D>       data;
    ap_uint<(D+7)/8> keep;
    ap_uint<(D+7)/8> strb;
    ap_uint<U>       user;
    ap_uint<1>       last;
    ap_uint<TI>      id;
    ap_uint<TD>      dest;
  };


void test1(hls::stream<AXI_24_VALUE> &in_video, hls::stream<AXI_24_VALUE> &out_video) {

   AXI_24_VALUE out;
   AXI_24_VALUE in;
   while(!in_video.empty()) {
      in = in_video.read();
      out.data = in.data;
      out.keep = 0xFF;
      out.last = in.last;
      out.strb = 0x0;
      out.id = 0;
      out.dest = 0;
      out_video.write(out);
   }
}


void test2(hls::stream<ap_axiu_type<24, 1, 1, 1> > &in_video, hls::stream<<ap_axiu_type<24, 1, 1, 1>> &out_video) {

   <ap_axiu_type<24, 1, 1, 1> out;
   <ap_axiu_type<24, 1, 1, 1> in;
   while(!in_video.empty()) {
      in = in_video.read();
      out.data = in.data;
      out.keep = 0xFF;
      out.last = in.last;
      out.strb = 0x0;
      out.id = 0;
      out.dest = 0;
      out_video.write(out);
   }
}
