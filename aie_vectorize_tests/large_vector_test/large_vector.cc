#ifndef __ALL_STAP_CC__
#define __ALL_STAP_CC__

#include "kernels.h"
#include "kernels.h"

#ifdef __VSI_AUTOVEC__
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "complex.h"
using namespace std;

   
void stap_compute_covariance_estimate(
	complex<float> * __restrict__ covariance,
	complex<float> * __restrict__ snapshot )
{
   int cell;
   // was 589,824 cfloats for perfect suite
   const size_t num_cov_elements = (TDOF*N_CHAN) * (TDOF*N_CHAN) * N_DOP * N_BLOCKS; // 65,536 cfloats = 524,288 bytes
   int i, j, k, conv_index, k_index;
   int k_i_index;
   int k_j_index;

   // my optimization
   #pragma clang loop vectorize(enable)
   #pragma loop distribute(enable)
	#pragma clang loop interleave_count(1)
   // for i=0:3
   for (k = 0; k < TRAINING_BLOCK_SIZE; k++) {
      k_index= k*8; 		// k_index = k*N_CHAN*TDOF;
      //i= 0;
      //j= 0;
      conv_index= 0; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 0; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 1;
      //j= 0;
      conv_index= 8; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 1; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 1;
      //j= 1;
      conv_index= 9; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 1; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 2;
      //j= 0;
      conv_index= 16; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 2; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 2;
      //j= 1;
      conv_index= 17; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 2; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 2;
      //j= 2;
      conv_index= 18; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 2; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 2; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 3;
      //j= 0;
      conv_index= 24; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 3; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 3;
      //j= 1;
      conv_index= 25; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 3; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 3;
      //j= 2;
      conv_index= 26; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 3; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 2; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
      //i= 3;
      //j= 3;
      conv_index= 27; 		// conv_index = i*N_CHAN*TDOF + j;
      k_i_index= k_index+ 3; 		// k_i_index=k*N_CHAN*TDOF+i;
      k_j_index= k_index+ 3; 		// k_j_index= k*N_CHAN*TDOF+j;
      covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
	}
   // for i= 4:5
   // for (k = 0; k < TRAINING_BLOCK_SIZE; k++) {
   //    k_index= k*8; 		// k_index = k*N_CHAN*TDOF;
   //    //i= 4;
   //    //j= 0;
   //    conv_index= 32; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 4; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 4;
   //    //j= 1;
   //    conv_index= 33; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 4; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 4;
   //    //j= 2;
   //    conv_index= 34; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 4; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 2; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 4;
   //    //j= 3;
   //    conv_index= 35; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 4; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 3; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 4;
   //    //j= 4;
   //    conv_index= 36; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 4; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 4; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 5;
   //    //j= 0;
   //    conv_index= 40; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 5; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 5;
   //    //j= 1;
   //    conv_index= 41; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 5; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 5;
   //    //j= 2;
   //    conv_index= 42; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 5; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 2; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 5;
   //    //j= 3;
   //    conv_index= 43; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 5; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 3; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 5;
   //    //j= 4;
   //    conv_index= 44; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 5; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 4; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 5;
   //    //j= 5;
   //    conv_index= 45; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 5; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 5; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);  
	// }
   // // for i=6
   // for (k = 0; k < TRAINING_BLOCK_SIZE; k++) {
   //    k_index= k*8; 		// k_index = k*N_CHAN*TDOF;
   //    //i= 6;
   //    //j= 0;
   //    conv_index= 48; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 6;
   //    //j= 1;
   //    conv_index= 49; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 6;
   //    //j= 2;
   //    conv_index= 50; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 2; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 6;
   //    //j= 3;
   //    conv_index= 51; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 3; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 6;
   //    //j= 4;
   //    conv_index= 52; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 4; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 6;
   //    //j= 5;
   //    conv_index= 53; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 5; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 6;
   //    //j= 6;
   //    conv_index= 54; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 6; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 6; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   // }
   // // for i=7
   // for (k = 0; k < TRAINING_BLOCK_SIZE; k++) {
   //    k_index= k*8; 		// k_index = k*N_CHAN*TDOF;
   //    //i= 7;
   //    //j= 0;
   //    conv_index= 56; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 0; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 1;
   //    conv_index= 57; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 1; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 2;
   //    conv_index= 58; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 2; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 3;
   //    conv_index= 59; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 3; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 4;
   //    conv_index= 60; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 4; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 5;
   //    conv_index= 61; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 5; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 6;
   //    conv_index= 62; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 6; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);
   //    //i= 7;
   //    //j= 7;
   //    conv_index= 63; 		// conv_index = i*N_CHAN*TDOF + j;
   //    k_i_index= k_index+ 7; 		// k_i_index=k*N_CHAN*TDOF+i;
   //    k_j_index= k_index+ 7; 		// k_j_index= k*N_CHAN*TDOF+j;
   //    covariance[conv_index] += snapshot[k_i_index] * cconj(snapshot[k_j_index]);   
	// }
  
}

#else

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
//#include "all_stap.cpp"
#include "large_vector_vectorized.cpp"  // this is the name of the VSI output generated vectorized source file

// INTERNAL_ITERATION_NUM = dops * blocks = 512*2=1024
// However, if using multiple AIE and wanting latency, divide that number by # of AIE used
// For example with 128 AIE used for STAP, we would change 1024 to 8
#define INTERNAL_ITERATION_NUM 1  // 1024 full value but divided by # AIE so latency shown in sim is correct.


void large_vector_top(
   // snapshot input from RDMA
   input_stream_float  * snapshot_stream_in,
	// outputs from this kernel
   output_window_float * final_out_data_window_out) {
   // int latency = get_cycles();
	float gamma_buf;

   
	for ( int internal_iteration = 0 ; internal_iteration < INTERNAL_ITERATION_NUM; internal_iteration++) {
		for (int i = 0; i < ( sizeof(snapshot_in_buffer)/ sizeof(float) ); i++ ) {
			snapshot_in_buffer[i] = readincr(snapshot_stream_in);
		}
		for (int i = 0; i < ( sizeof(covariance_buffer)/ sizeof(float) ); i++ ) {
			covariance_buffer[i] = 0.0f;
		}

		cfloat* covariancecomplex = (cfloat*)covariance_buffer;
		cfloat* snapshotcomplex_in = (cfloat*)snapshot_in_buffer;

		// calling the vectorized stap_compute_covariance_estimate
		stap_compute_covariance_estimate(covariancecomplex, snapshotcomplex_in);
      
			window_acquire(final_out_data_window_out);
	//		// ---------------------------------------------------------------------
   //
	//		// Send final data out
			for (int i = 0; i < ( sizeof(covariance_buffer)/ sizeof(float) ); i++ ) {
				window_writeincr(final_out_data_window_out, covariance_buffer[i]);
			}
			window_release(final_out_data_window_out);
   // 
   // ---------------------------------------------------------------------
	}
   // latency = get_cycles() - latency;
}

#endif
#endif
