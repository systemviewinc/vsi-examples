#include <hls_stream.h>

void source_sink(hls::stream<unsigned int> &in, hls::stream<unsigned int> &out) {
  // sendo out 4 k bytes then loop everything coming in
  for (int i = 0 ; i < 1024; i ++ ) out.write(i);
  while (1) {
    out.write(in.read());
  }
}

void source(hls::stream<unsigned int> &out) {
 // sendo out 4 k bytes then exit
  for (int i = 0 ; i < 1024; i ++ ) 
#pragma HLS pipeline II=1
    out.write(i);
}

unsigned int buff[1024];

void sink(hls::stream<unsigned int>&in) {
  while (1) {
    for (int i = 0 ; i < 1024; i ++ ) 
#pragma HLS pipeline II=1
      buff[i] = in.read();
  }
}
