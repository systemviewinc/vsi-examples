#include <cardano.h>

void ViterbiMain(input_stream_int32 * instream, output_stream_int32 * outstream1, output_stream_int32 * outstream2) {
  int32 i;
  int16 num_parity_bits=2;
  int16 num_bits;
  num_bits = readincr(instream);
  writeincr(outstream2, num_bits);
  //input file contains number of bits plus the bits
  for (i=0;i<num_bits;i++) {
    int32 data = readincr(instream);
    writeincr(outstream2, data);
  }
  //outputs_.resize(1 << constraint_); --> size= 1<<7 (128)
  // instead of string "01" we use two int so the size of our output_ will be 256
  int32 outputs_[256];
  int32 rev_polynomials[2]; // Reverse of polynomials when number of bits is 7 (constraint)
  /////// initialize outputs_
  for (i = 0; i < 128; i++) {
	  rev_polynomials[0] = 109;
	  rev_polynomials[1] = 79;
	  for (int32 j = 0; j < num_parity_bits; j++) {
		  int32 input = i;
		  int32 output = 0;
		  for (int32 k = 0; k < 7; k++) {
			  output ^= (input & 1) & (rev_polynomials[j] & 1);
			  rev_polynomials[j] >>= 1;
			  input >>= 1;
		  }
		  outputs_[(i*2)+j] = output ? 1 : 0;
	 }
  }
  for (i=0;i<256;i++) {
	writeincr(outstream1, outputs_[i]);
  }
}
