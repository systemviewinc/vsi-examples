/**
 *
 * @brief  HLS Viterbi Decoder 
 *
 * 
 * In this example of Viterbi Decoder:
 *      constrain length = 7
 *      first reverse polynominal = 109
 *      second reverse polynominal = 79
 *      number of parity bits = 2
 *      output size = 256
 * 
 * outputs is (1 << constraint_) --> size = 1<<7 (128)
 * instead of string "01" we use two integers so the size of the output will be 256
 */

#define POLYNOMINAL_REV_0 109
#define POLYNOMINAL_REV_1 79
#define OUTPUT_SIZE 256
#define PARITY_BITS 2
#define INT_MAX 2147483647

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <utility>
#include <vector>


void Viterbi_HLS(int in_arr[1024], int out_arr[1024]){

    int num_parity_bits = PARITY_BITS;
    int num_bits;
    int outputs_[OUTPUT_SIZE];
    int rev_polynomials[2]; // Reverse of polynomials
    int j=0;
    int i,l;
    int constraint;
    int polynomials[2];
    int bits[50]= {0};
    int out_arr_[1024];

  //input data contains number of bits plus the bits
  num_bits = in_arr[0];
  for (i = 0; i < num_bits; i++){
      bits[i] = in_arr[i+1];
  }
    // initializing output
  for (i = 0; i < 128; i++){
    rev_polynomials[0] = POLYNOMINAL_REV_0;
    rev_polynomials[1] = POLYNOMINAL_REV_1;
    for (int j = 0; j < num_parity_bits; j++){
      int input = i;
      int output = 0;
      for (int k = 0; k < 7; k++){
        output ^= (input & 1) & (rev_polynomials[j] & 1);
        rev_polynomials[j] >>= 1;
        input >>= 1;
      }
      outputs_[(i * 2) + j] = output ? 1 : 0;
    }
  }

  // constant parameters
	  constraint = 7;
	  polynomials[0] = 91;
	  polynomials[1] = 121;
	  //////// start of decoder part
	  //1 << (constraint_ - 1) is 64 when the constraint_ is 7
	  int path_metrics[64];
	  int trellis_index =0;
	  int 	trellis_array[64*64];
	  int trellis_array_index=0;
	  int current_bits[2]; //2 bits due to 2 number of parity bits
	  for (i=0;i<64;i++) {
		  path_metrics[i] = INT_MAX;
	  }
	  path_metrics[0] = 0;
	  for (i = 0; i < num_bits; i += num_parity_bits) {
		  if (i<num_bits -1){
			  current_bits[0] = bits[i];
			  current_bits[1] = bits[i+1];
		  } else { //if i == num_bits -1
			  current_bits[0] = bits[i];
			  current_bits[1] = 0;
		  }
		  /////Start of UpdatePathMetrics
		  int new_path_metrics[64];
		  int new_trellis_column[64];
		  //PathMetric.size =64
		  for (j = 0; j < 64; j++) {
			  ///// start of PathMetric
			  //constraint_ - 2 is 5
			  // s = (i*2)% 32;
			  int s = (j & ((1 << 5) - 1)) << 1;
			  int source_state1 = s | 0;
			  int source_state2 = s | 1;
			  int pm1 = path_metrics[source_state1];
			  if (pm1 < INT_MAX) {
				  //// start of BranchMetric
				  ////start of Output(source_state, target_state >> (constraint_ - 2));
				  int target_output = j >> 5;
				  int index = (source_state1 | (target_output<<6));
				  int output[2];
				  output[0]= outputs_[index*2];
				  output[1]= outputs_[(index*2)+1];
				  ///HammingDistance current_bits and output
				  int distance = 0;
				  for (int k = 0; k < 2; k++) {
					distance += current_bits[k] != output[k];
				  }
				  pm1 += distance;
			    }
			  int pm2 = path_metrics[source_state2];
			  if (pm2 < INT_MAX) {
				  int target_output = j >> 5;
				  int index = (source_state2 | (target_output<<6));
				  int output[2];
				  output[0]= outputs_[index*2];
				  output[1]= outputs_[(index*2)+1];
				  int distance = 0;
				  for (int k = 0; k < 2; k++) {
					distance += current_bits[k] != output[k];
				  }
				  pm2 += distance;
				}
			  ////// end of BranchMetric
			  if (pm1 <= pm2) {
				  new_path_metrics[j] = pm1;
				  new_trellis_column[j] = source_state1;
			    } else {
					new_path_metrics[j] = pm2;
					new_trellis_column[j] = source_state2;
			    }
		  }
		  for (l=0;l<64;l++) {
			path_metrics[l] = new_path_metrics[l];
			trellis_array_index = (trellis_index * 64) + l;
			trellis_array[trellis_array_index] = new_trellis_column[l];
		  }
		  trellis_index++;
		  if (trellis_index> 63) {
			  trellis_index =0;
		  }
		  ///////End of UpdatePathMetrics
	  }
	  // Traceback.
	  int decoder_state= INT_MAX;
	  for (l=0;l<64;l++) {
		  if (path_metrics[l] < decoder_state) {
			  decoder_state= l;
		  }
	  }
	  int decoded[50];
	  j=0;
	  int index= (num_bits/num_parity_bits) - 1;
	  int max_iteration = (num_bits/num_parity_bits);
	  for (j=0; j<max_iteration; j++) {
		  int shifted= decoder_state >> 5;
		  if (shifted > 0){
			  decoded[j] =1;
		  } else {
			  decoded[j] =0;
		  }
	      decoder_state = trellis_array[(index*64) + decoder_state];
	      index--;
	    }
        i= 0;
	  for (j=max_iteration-1;j> (constraint-2); j--) {
          out_arr_[i] = decoded[j];
          i++;
	  	  }
    // write zero till the end of 1024 data
    for (j=i; j<1024; j++) {
            out_arr_[j] = 0;
	  	  }
    // Write the decoded bits out
    for (j=0; j<1024; j++) {
            out_arr[j] = out_arr_[j] ;
	}
};
