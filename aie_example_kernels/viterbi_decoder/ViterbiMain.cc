/**
 *
 * @brief  Viterbi Decoder implemented in 2 kernels of AIe
 *
 * The first kernel (ViterbiMain) receives the input data (instream)
 * and split it to number of data and the data 
 * and send them to the next kernel as outstream2
 * ViterbiMain kernel also initializes the output data
 * 
 * In this example of Viterbi Decoder:
 *      constrain_length = 7
 *      reverse_polynominal_0 = 109
 *      reverse_polynominal_1 = 79
 *      num_parity_bits = 2
 *      output_size = 256
 * 
 * outputs is (1 << constraint_) --> size = 1<<7 (128)
 * instead of string "01" we use two integers so the size of the output will be 256
 */

#define POLYNOMINAL_REV_0 109
#define POLYNOMINAL_REV_1 79
#define OUTPUT_SIZE 256
#define PARITY_BITS 2


#include <cardano.h>

void ViterbiMain(input_stream_int32 *instream,
                output_stream_int32 *outstream1,
                output_stream_int32 *outstream2){
  int32 i;
  int16 num_parity_bits = PARITY_BITS;
  int16 num_bits;
  int32 outputs_[OUTPUT_SIZE];
  int32 rev_polynomials[2]; // Reverse of polynomials

  //input data contains number of bits plus the bits
  num_bits = readincr(instream);
  writeincr(outstream2, num_bits);
  for (i = 0; i < num_bits; i++){
    int32 data = readincr(instream);
    writeincr(outstream2, data);
  }
  
  // initializing output
  for (i = 0; i < 128; i++){
    rev_polynomials[0] = POLYNOMINAL_REV_0;
    rev_polynomials[1] = POLYNOMINAL_REV_1;
    for (int32 j = 0; j < num_parity_bits; j++){
      int32 input = i;
      int32 output = 0;
      for (int32 k = 0; k < 7; k++){
        output ^= (input & 1) & (rev_polynomials[j] & 1);
        rev_polynomials[j] >>= 1;
        input >>= 1;
      }
      outputs_[(i * 2) + j] = output ? 1 : 0;
    }
  }
  for (i = 0; i < OUTPUT_SIZE; i++){
    writeincr(outstream1, outputs_[i]);
  }
}
