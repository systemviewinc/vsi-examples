#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <hls_stream.h>
#include <ap_utils.h>

#include "ap_axi_sdata.h"

#define BIG_ARRAY_SIZE 32

#define TEST_DATA_SIZE 32
/**
 * @brief Stream decoder
 *
 * @param codestream - input encoded stream
 * @param codeword - input code world
 * @param dcdstream - output decoded output stream
 */

void val2stream_dcd (
    hls::stream<int> &codestream,
    int codeword,
    hls::stream<ap_axis<32, 0, 0, 0> > &dcdstream)
{

  int inval_rd = codeword;
  unsigned int counter = TEST_DATA_SIZE;
  while (counter--) {
    int t2 = codestream.read();
    ap_axis<32, 0, 0, 0> t_out;

    t_out.data = t2 ^ inval_rd;
    t_out.keep = -1; //Enabling all bytes
    t_out.last = counter ? 0 : 1;
    dcdstream.write(t_out);
  }
}
