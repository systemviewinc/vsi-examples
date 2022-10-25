
#ifndef __ALL_STAP_CC__
#define __ALL_STAP_CC__

#include "kernels.h"

#ifdef __VSI_AUTOVEC__
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "_complex.h"
   
int index_i[36] ={0*128, 1*128, 1*128, 2*128, 2*128, 2*128, 3*128, 3*128, 3*128, 3*128, 4*128, 4*128, 4*128, 4*128, 4*128, 5*128, 5*128, 5*128, 5*128, 5*128, 5*128, 6*128, 6*128, 6*128, 6*128, 6*128, 6*128, 6*128, 7*128, 7*128, 7*128, 7*128, 7*128, 7*128, 7*128, 7*128} ;
int index_j[36] ={0*128, 0*128, 1*128, 0*128, 1*128, 2*128, 0*128, 1*128, 2*128, 3*128, 0*128, 1*128, 2*128, 3*128, 4*128, 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128} ;
int out_index[36] = {0, 8, 9, 16, 17, 18, 24, 25, 26, 27, 32, 33, 34, 35, 36, 40, 41, 42, 43, 44, 45, 48, 49, 50, 51, 52, 53, 54, 56, 57, 58, 59, 60, 61, 62, 63};

int cconj_index[64] = {
		1, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		
        1, 		1, 		0, 		0, 		0, 		0, 		0, 		0, 		
        1, 		1, 		1, 		0, 		0, 		0, 		0, 		0, 		
        1, 		1, 		1, 		1, 		0, 		0, 		0, 		0, 		
        1, 		1, 		1, 		1, 		1, 		0, 		0, 		0, 		
        1, 		1, 		1, 		1, 		1, 		1, 		0, 		0, 		
        1, 		1, 		1, 		1, 		1, 		1, 		1, 		0, 		
        1, 		1, 		1, 		1, 		1, 		1, 		1, 		1		
	};
int read_index[(N_CHAN*TDOF)*(N_CHAN*TDOF)] = {
	000,8,16,24,32, 40,
	48, 56, 01, 9,  17, 25,  33,  41,  49,  57,  02,  10,  18,  26,  34,  42,  50,  58,  03,
	11, 19, 27, 35, 43, 51,  59,  4,   12,  20,  28,  36,  44,  52,  60,   5,  13,  21,  29,
	37, 45, 53, 61, 6,  14,  22,  30,  38,  46,  54,  62,   7,  15,  23,  31,  39,  47,  55, 63
};

void stap_compute_covariance_estimate(
	_complex<float> * __restrict__ covariance,
	_complex<float> * __restrict__ covariance_c,
	_complex<float> * __restrict__ __attribute__((aligned(16))) snapshot  )
{
    int cell;
    const size_t num_cov_elements = (TDOF*N_CHAN) * (TDOF*N_CHAN) * N_DOP * N_BLOCKS;
    _complex<float> zero(0.0f, 0.0f);
    _complex<float> cc[N_CHAN*TDOF*N_CHAN*TDOF] __attribute__((aligned(16)));
	
    /*
     * It is assumed for simplicity that the training block
     * size evenly divides the range swath.
     */
    /*assert(N_RANGE % TRAINING_BLOCK_SIZE == 0); */


    int i, j, k;
	// will put snap and cov in different banks
    /* Outer products are accumulated over a full block. */
    for (i = 0; i < 36; ++i) {
	    /* Exploit conjugate symmetry by only accumulating along
	     * the diagonal and below. */
	    // #pragma clang vectorize loop interleave_count(1)
	    _complex<float> x(0.0,0.0);
	    _complex<float> *  __attribute__((aligned(16))) cA = &snapshot[index_i[i]];
	    _complex<float> * __attribute__((aligned(16)))  cB = &snapshot[index_j[i]];
#pragma clang loop vectorize(enable)
	    for (k = 0; k < TRAINING_BLOCK_SIZE; k++) {
		    x += cA[k] * cconj(cB[k]);
	    }
	    // conv_index = i*N_CHAN*TDOF + j;
	    covariance[out_index[i]] = (x);
	    // cc[out_index[i]] = (x);
    }
    
    /*
     * The covariance matrices are conjugate symmetric, so
     * we copy the conjugate of the lower triangular portion
     * into the upper triangular portion.
     */
	    // for (i = 0; i < N_CHAN*TDOF; ++i)
        //     {
		//     for (j = i+1; j < N_CHAN*TDOF; ++j)
		// 	    {
		// 		    const _complex<float> x = covariance[j*N_CHAN*TDOF+i];
		// 		    conv_index = i*N_CHAN*TDOF + j;
		// 		    covariance[conv_index]= cconj(x);
		// 	    }
        //     }

    /*
     * Normalize the covariance matrices by dividing by the
     * number of training samples.
     */
    // for (i = 0; i < N_CHAN*TDOF*N_CHAN*TDOF; ++i) {
    //     cc[i] = covariance[read_index[i]];
    // }
#pragma clang loop vectorize(enable)
    for (i = 0; i < N_CHAN*TDOF*N_CHAN*TDOF; ++i) {
	    //	    for (j = 0; j < N_CHAN*TDOF; ++j) {
	    int conv_index = i;//*N_CHAN*TDOF + j;
	    _complex<float> x = covariance[i];
	    _complex<float> y = cconj(covariance_c[read_index[i]]);
	    if (!cconj_index[i]) {
		    x = y;
	    }
	    covariance[i] = x* sample_norm;
	    // }
    }
//     for (i = 0; i < N_CHAN*TDOF; ++i) {
// #pragma clang loop vectorize(enable)
// 	    for (j = 0; j < N_CHAN*TDOF; ++j) {
// 		    int conv_index = i*N_CHAN*TDOF + j;
// 		    if (j>=i+1){
// 			    const _complex<float> x = cc[j*N_CHAN*TDOF+i];
// 			    covariance[conv_index] = cconj(x);
// 		    }
// 		    covariance[conv_index] = covariance[conv_index] * sample_norm;
// 	    }
//     }
}

#else

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "covariance.cpp"  // this is the name of the VSI output generated vectorized source file

// From Steve:
// INTERNAL_ITERATION_NUM = dops * blocks = 512*2=1024
// However, if using multiple AIE and wanting latency, divide that number by # of AIE used
// For example with 128 AIE used for STAP, we would change 1024 to 8
#define INTERNAL_ITERATION_NUM 1  // 1024 full value but divided by # AIE so latency shown in sim is correct.


void covariance_top(
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

		struct class__complex* covariance_complex = (struct class__complex*)covariance_buffer;
		struct class__complex* snapshot_complex_in = (struct class__complex*)snapshot_in_buffer;
	    struct class__complex* final_data_complex_out = (struct class__complex*)final_data_out_buffer;

		// calling the vectorized stap_compute_covariance_estimate
		stap_compute_covariance_estimate(covariance_complex, 
		covariance_complex, snapshot_complex_in);
      
			window_acquire(final_out_data_window_out);
		// Send final data out
			for (int i = 0; i < ( sizeof(covariance_buffer)/ sizeof(float) ); i++ ) {
				window_writeincr(final_out_data_window_out, covariance_buffer[i]);
			}
			window_release(final_out_data_window_out);
   // ---------------------------------------------------------------------
	}
   // latency = get_cycles() - latency;
}

#endif
#endif
