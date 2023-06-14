#include "stream_and_mem_test.h"

// HLS process data functions

// in VSI, use execution trigger on instream
void process_stream_and_mem(hls::stream<ap_uint<DATA_BIT_WIDTH> >& instream, 
    hls::stream<ap_uint<DATA_BIT_WIDTH> >& outstream,
    ap_uint<DATA_BIT_WIDTH>* mem)
{
    ap_uint<DATA_BIT_WIDTH> buf[N_SAMPLES];

    for(int i = 0; i < N_SAMPLES; i++){
        buf[i] = instream.read();
        mem[i] = buf[i]+1000;
    }

    for(int i = 0; i < N_SAMPLES; i++){
        outstream.write(buf[i]+100);
    }
}

void process_stream_packet(hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& instream, 
    hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& outstream)
{
    ap_axiu<DATA_BIT_WIDTH,0,0,0> strm_pkt;
    for(int i = 0; i < N_SAMPLES; ++i){
        strm_pkt = instream.read();
        strm_pkt.data = strm_pkt.data*2;
        strm_pkt.keep = -1;
        outstream.write(strm_pkt);
    }
}


