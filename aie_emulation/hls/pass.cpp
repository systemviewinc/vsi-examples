// #include "ap_axi_sdata.h"
// #include "ap_int.h"
// #include "hls_stream.h"

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <hls_stream.h>
#include <ap_utils.h>

#include "ap_axi_sdata.h"

#define BIG_ARRAY_SIZE 32

/**
 * @brief Stream pass thru
 *
 * @param ins
 * @param outd
 */
// void pass_thru_streaming(  hls::stream<int> &ins, hls::stream<int> &outd)
void pass_thru_streaming (
    hls::stream<int> &ins,
    hls::stream<ap_axis<32, 0, 0, 0> > &outd)
{
  unsigned int counter = 1;
  while (true)
  {
    // ap_axis<32, 0, 0, 0> t2 = ins.read();
    int t2 = ins.read();
    // Packet for Output
    ap_axis<32, 0, 0, 0> t_out;

    t_out.data = t2;
    // t_out.data = t2.data;
    t_out.keep = -1; //Enabling all bytes
    t_out.last = (counter == BIG_ARRAY_SIZE) ? 1 : 0;
    
    outd.write(t_out);
    
    counter = (counter == BIG_ARRAY_SIZE) ? 1 : counter+1;
  }
}




