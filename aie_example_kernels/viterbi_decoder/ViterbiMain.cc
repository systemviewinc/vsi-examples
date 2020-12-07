/**
 *
 * @brief  Viterbi Decoder implemented in 2 kernels of AIe
 *
 * The first kernel (ViterbiMain) receives the input data (instream)
 * and split it to number of data and the encoded data 
 * and send them to the next kernel as data_to_decoder
 * ViterbiMain kernel also initializes the output data
 * and send them to the next kernel as initialized_output
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


#include <adf.h>

void ViterbiMain(input_stream_int32 *instream,
                output_stream_int32 *initialized_output,
                output_stream_int32 *data_to_decoder){
  int32 i;
  int16 num_parity_bits = PARITY_BITS;
  int16 num_bits;
  int32 outputs_[OUTPUT_SIZE];
  int32 rev_polynomials[2]; // Reverse of polynomials

  //input data contains number of bits plus the bits
  num_bits = readincr(instream);
  writeincr(data_to_decoder, num_bits);
  for (i = 0; i < num_bits; i++){
    int32 data = readincr(instream);
    writeincr(data_to_decoder, data);
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
    writeincr(initialized_output, outputs_[i]);
  }
}
