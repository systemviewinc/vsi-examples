#include <adf.h>
#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <utility>
#include <vector>

void Decoder(input_stream_int32 * init_data,
			input_stream_int32 * in_bits,
			output_stream_int32 * decodded_bits) {
	  int32 j=0;
	  int32 i,l;
	  int32 constraint;
	  int32 polynomials[2];
	  int32 num_bits;
	  int32 bits[50]= {0};
	  int32 outputs_[256];
	  num_bits = readincr(in_bits);
	  for (i=0;i<num_bits;i++) {
		bits[i] = readincr(in_bits);
	  };
	  //Initial outputs
  	  for (i=0;i<256;i++) {
  		  outputs_[i] = readincr(init_data);
//  		  writeincr(decodded_bits, outputs_[i]);
  	  }
  // constant parameters
	  constraint = 7;
	  polynomials[0] = 91;
	  polynomials[1] = 121;
	  int32 num_parity_bits = 2;
	  //////// start of decoder part
	  //1 << (constraint_ - 1) is 64 when the constraint_ is 7
	  int32 path_metrics[64];
	  int32 trellis_index =0;
	  int8 	trellis_array[64*64];
	  int32 trellis_array_index=0;
	  int32 current_bits[2]; //2 bits due to 2 number of parity bits
	  for (i=0;i<64;i++) {
		  path_metrics[i] = std::numeric_limits<int32>::max();
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
		  int32 new_path_metrics[64];
		  int32 new_trellis_column[64];
		  //PathMetric.size =64
		  for (j = 0; j < 64; j++) {
			  ///// start of PathMetric
			  //constraint_ - 2 is 5
			  // s = (i*2)% 32;
			  int32 s = (j & ((1 << 5) - 1)) << 1;
			  int32 source_state1 = s | 0;
			  int32 source_state2 = s | 1;
			  int32 pm1 = path_metrics[source_state1];
			  if (pm1 < std::numeric_limits<int32>::max()) {
				  //// start of BranchMetric
				  ////start of Output(source_state, target_state >> (constraint_ - 2));
				  int32 target_output = j >> 5;
				  int32 index = (source_state1 | (target_output<<6));
				  int32 output[2];
				  output[0]= outputs_[index*2];
				  output[1]= outputs_[(index*2)+1];
				  ///HammingDistance current_bits and output
				  int32 distance = 0;
				  for (int8 k = 0; k < 2; k++) {
					distance += current_bits[k] != output[k];
				  }
				  pm1 += distance;
			    }
			  int32 pm2 = path_metrics[source_state2];
			  if (pm2 < std::numeric_limits<int32>::max()) {
				  int32 target_output = j >> 5;
				  int32 index = (source_state2 | (target_output<<6));
				  int32 output[2];
				  output[0]= outputs_[index*2];
				  output[1]= outputs_[(index*2)+1];
				  int32 distance = 0;
				  for (int8 k = 0; k < 2; k++) {
					distance += current_bits[k] != output[k];
				  }
				  pm2 += distance;
				}
			  ////// end of BranchMetric
			  if (pm1 <= pm2) {
//				  writeincr(decodded_bits, pm1);
				  new_path_metrics[j] = pm1;
				  new_trellis_column[j] = source_state1;
			    } else {
//			    	writeincr(decodded_bits, pm2);
					new_path_metrics[j] = pm2;
					new_trellis_column[j] = source_state2;
			    }
			  /////End of PathMetric
		  }
		  for (l=0;l<64;l++) {
			path_metrics[l] = new_path_metrics[l];
//			trellis[trellis_index][l] = new_trellis_column[l];
//			writeincr(decodded_bits, trellis[trellis_index][l]);
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
	  int32 decoder_state= std::numeric_limits<int32>::max();
	  for (l=0;l<64;l++) {
//		  writeincr(decodded_bits, l);
//		  writeincr(decodded_bits, path_metrics[l]);
		  if (path_metrics[l] < decoder_state) {
			  decoder_state= l;
		  }
	  }
	  int32 decoded[50];
	  j=0;
	  int32 index= (num_bits/num_parity_bits) - 1;
	  int32 max_iteration = (num_bits/num_parity_bits);
	  for (j=0; j<max_iteration; j++) {
//		  decoded[j]= decoder_state >> 5 ? 1 : 0;
		  int32 shifted= decoder_state >> 5;
		  if (shifted > 0){
			  decoded[j] =1;
		  } else {
			  decoded[j] =0;
		  }
//	      decoder_state = trellis[index][decoder_state];
	      decoder_state = trellis_array[(index*64) + decoder_state];
	      index--;
	    }
	  // Write the decoded bits out
	  for (j=max_iteration-1;j> (constraint-2); j--) {
		  writeincr(decodded_bits, decoded[j]);
	  	  }
};
