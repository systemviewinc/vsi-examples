
#ifndef __SUBSTITUTION_CC__
#define __SUBSTITUTION_CC__

#include "kernels.h"
#ifdef __VSI_AUTOVEC__
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "_complex.h"
int accum_out_index [] = {0,1,2,2,3,3,3,4,4,4,4,5,5,5,5,5,6,6,6,6,6,6,7,7,7,7,7,7,7,8,8,8};
int addw_out_index [] =  {0,1,8,2,8,8,3,8,8,8,4,8,8,8,8,5,8,8,8,8,8,6,8,8,8,8,8,8,7,8,8,8};
int accum_in2_index[]  = {0,0,0,1,0,1,2,0,1,2,3,0,1,2,3,4,0,1,2,3,4,5,0,1,2,3,4,5,6,7,7,7};
int accum_in1_index[]  = {0,1,2,10,3,11,19,4,12,20,28,5,13,21,29,37,6,14,22,30,38,46,7,15,23,31,39,47,55,0,0,0};
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void forward_and_back_substitution(
           _complex<float> * __restrict__ adaptive_weights,
            _complex<float> * __restrict__ cholesky_factors,
          _complex<float> * __restrict__ steering_vectors,
           float * __restrict__ gamma)
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
{

	int i, j, k, lc;
	_complex<float> accum[N_CHAN*TDOF*2];
	/* First apply forward substitution */
	lc = 0 ;
	float Rii_inv[ N_CHAN*TDOF*2] ;
	/* moved inverse cholesky out */
	for (i = 0 ; i <  N_CHAN*TDOF; i++) {
		Rii_inv[i] = 1.0f / cholesky_factors[i*N_CHAN*TDOF+i].real();
		accum[i] = _complex<float>(0.0f,0.0f);
	}
	_complex<float> adw[N_CHAN*TDOF*2];
	/* move accoumulator out */
	for (i = 0 ; i < 32; i++) {
		accum[accum_out_index[i]] += cconj(cholesky_factors[accum_in1_index[i]]) * adaptive_weights[accum_in2_index[i]];
		adaptive_weights[addw_out_index[i]] = (steering_vectors[accum_out_index[i]] - accum[accum_out_index[i]]) * Rii_inv[accum_out_index[i]];
		// cout << " i = " << i
		//      << " addw_out_index[i] "  << addw_out_index[i]
		//      << " accum_out_index[i] " << accum_out_index[i]
		//      << " accum_in1_index[i] "  << accum_in1_index[i]
		//      << " accum_in2_index[i] "  << accum_in2_index[i] << "\n";
	}
// #pragma clang loop unroll_count(2)
// 	for (i = 0; i < N_CHAN*TDOF; ++i) {
// 		//const float Rii_inv = 1.0f / cholesky_factors[i*N_CHAN*TDOF+i].real();
// #pragma clang vectorize loop interleave_count(1)
// 		_complex<float> accum1(0.0f,0.0f);

// 		cout << " i = " << i << "\n";
// 		for (j = 0; j < i;  ++j) {
// 			/*
// 			 * Use the conjugate of the upper triangular entries
// 			 * of cholesky_factors as the lower triangular entries.
// 			 */
// 			cout << "loop1 ii lc = " << lc++ << " i = " << i << " j = " << j
// 			      << " [" << j*N_CHAN*TDOF+i << "] " << "\n";
// 			_complex<float> prod(0.0f, 0.0f);
// 			prod = cconj(cholesky_factors[j*N_CHAN*TDOF+i]) * adaptive_weights[j];
// 			accum1 += prod;

// 		}	
// 		adaptive_weights[i] = (steering_vectors[i] - accum1) * Rii_inv[i];
// 		cout << " adaptive_weights[i] " << adaptive_weights[i] << " adw[i] " << adw[i] << "\n"; 
// 	}
	//cout << "loop 1 k = " << k << "\n";
	/* And now apply back substitution */
	// #pragma clang loop unroll_count(2)
	lc = 0;
	for (j = N_CHAN*TDOF-1; j >= 0; --j) {
		//const float Rjj_inv = 1.0f / cholesky_factors[j*N_CHAN*TDOF+j].real();

		_complex<float> accum2(0.0f,0.0f);
		for (k = 0; k < N_CHAN*TDOF; ++k) {

			// cout << "loop2 ii lc = " << lc++ << " j = " << j << " k = " << k
			    //  << " [ " << j*N_CHAN*TDOF+k << "]" << "\n";
			_complex<float> prod = cholesky_factors[j*N_CHAN*TDOF+k] * adaptive_weights[k];
			if (k<j+1)
				prod =_complex<float>(0.0f,0.0f);

			accum2 += prod;
		}
		adaptive_weights[j] = (adaptive_weights[j] - accum2) * Rii_inv[j];
	}
	/*
	 * calculating the gama in forward_and_back_substitution
	 * to reduce the number of AIEs
	 */

	_complex<float> accum_gamma(0.0f,0.0f);



	for (i = 0; i < N_CHAN*TDOF; ++i) {
		const _complex<float> prod = cconj(adaptive_weights[i]) * steering_vectors[i];
		accum_gamma += prod;
	}

	/*
	 * In exact arithmetic, accum should be a real positive
	 * scalar and thus the imaginary component should be zero.
	 * However, with limited precision that may not be the case,
	 * so we take the magnitude of accum.  Also, gamma is a
	 * normalization scalar and thus we take the inverse of
	 * the computed inner product, w*v.
	 */
	// *gamma = sqrtf(accum_gamma.real()*accum_gamma.real() + accum_gamma.imag()*accum_gamma.imag());
   *gamma = sqrt(cnorm(accum_gamma));
	if (*gamma > 0) {
		*gamma = 1.0f / *gamma;
	} else {
		*gamma = 1.0f;
	}

}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void complex_inner_product(
    _complex<float> *outvalue,
     _complex<float> *lhs,
     _complex<float> *rhs,
     float  *gamma
    )
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
{
    int i,ci;
    for ( ci = 0; ci < TRAINING_BLOCK_SIZE; ci++) {
        _complex<float> accum(0.0f,0.0f);
        for (i = 0; i < CH_TDOF; ++i)
        {
            const _complex<float> prod = 
                cconj(lhs[i])* rhs[ci*CH_TDOF +i];
            accum += prod;
        }
        accum =accum * (*gamma); 
        outvalue[ci] = accum;
    }
    // _complex<float> prod_array[TRAINING_BLOCK_SIZE * CH_TDOF];
    // for (i = 0; i < CH_TDOF; ++i)
    // {
    //   for ( ci = 0; ci < TRAINING_BLOCK_SIZE; ci++)
    //   {
    //     prod_array[ci*CH_TDOF +i] = cconj(lhs[i])* rhs[ci*CH_TDOF +i];
    //   }
    // }

    // for ( ci = 0; ci < TRAINING_BLOCK_SIZE; ci++)
    // {
    //   _complex<float> accum(0.0f,0.0f);
    //   for (i = 0; i < CH_TDOF; ++i)
    //   {
    //     accum += prod_array[ci*CH_TDOF +i];
    //   }
    //   accum =accum * (*gamma); 
    //   outvalue[ci] = accum;
    // }
}


/** \brief Kernel 2: Weight Generation (Linear System Solver)
 *   
 *  \param[in]  snapshot          Vectorized snapshot of all channels for a given pulse (plus T_DOF-1 neighboring pulses).
 *  \param[in]  adaptive_weights  Array of (N_CHAN*TDOF) complex floats.
 *  \param[in]  cholesky_factors  Array of (N_CHAN*TDOF)^2 complex floats.
 *  \param[out] steering_vectors  Flattened array of steering vectors. Each steering vector is N_CHAN*TDOF complex floats
 *  \param[out] outvalue          Adaptive weights output. Array with length N_CHAN*TDOF of complex floats (each 2 floats).
 */



#else // not __VSI_AUTOVEC__


#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "substitution.cpp"

// From Steve:
// INTERNAL_ITERATION_NUM = dops * blocks = 512*2=1024
// However, if using multiple AIE and wanting latency, divide that number by # of AIE used
// For example with 128 AIE used for STAP, we would change 1024 to 8
#define INTERNAL_ITERATION_NUM 1  // 1024 full value but divided by # AIE so latency shown in sim is correct.

void compute_weights
(
  // snapshot input from RDMA
  input_stream_float  * snapshot_stream_in,
  // snapshot input from RDMA
  input_stream_float  * cholesky_stream_in,
  // steering_vectors input from RDMA
  input_window_float  * steering_vectors_window_in,
  // outputs from this kernel
    output_window_float * final_out_data_window_out
) {
  float gamma_buf;
  float *steering_vec_ptr = (float *)steering_vectors_window_in->ptr;
  printf("START of the top function!!!!!!!!!!!!!!! \n");

  for (int i = 0; i < ( sizeof(steering_vector_in_buffer)/ sizeof(float) ); i++ ) { 
    steering_vector_in_buffer[i] = steering_vec_ptr[i];
  }

  for ( int internal_iteration = 0 ; internal_iteration < INTERNAL_ITERATION_NUM; internal_iteration++) {
    for (int i = 0; i < ( sizeof(snapshot_in_buffer)/ sizeof(float) ); i++ ) {
      snapshot_in_buffer[i] = readincr(snapshot_stream_in);
    }
    for (int i = 0; i < ( sizeof(cholesky_factor_buffer)/ sizeof(float) ); i++ ) {
      cholesky_factor_buffer[i] = readincr(cholesky_stream_in);
    }

    struct class__complex* snapshot_complex_in = (struct class__complex*)snapshot_in_buffer;
    struct class__complex* cholesky_factor_complex = (struct class__complex*)cholesky_factor_buffer;

    // for (int sv=0; sv<N_STEERING; sv++ ) {
      window_acquire(final_out_data_window_out);
      // ---------------------------------------------------------------------
      struct class__complex* final_data_complex_out = (struct class__complex*)final_data_out_buffer;
      struct class__complex* adaptive_weights_complex_out = (struct class__complex*)adaptive_weights_out_buffer;
      struct class__complex* steering_vectors_complex_in = (struct class__complex*)(steering_vector_in_buffer);
      // calling the vectorized FB_Substitude
      forward_and_back_substitution(
          adaptive_weights_complex_out,
          cholesky_factor_complex,
          steering_vectors_complex_in,
          &gamma_buf);
      // calling the vectorized inner_complex_product function
      complex_inner_product(final_data_complex_out, adaptive_weights_complex_out, snapshot_complex_in, &gamma_buf);
      // ---------------------------------------------------------------------

      // Send final data out
      for (int i = 0; i < ( sizeof(final_data_out_buffer)/ sizeof(float) ); i++ ) {
        window_writeincr(final_out_data_window_out, final_data_out_buffer[i]);
      }
      window_release(final_out_data_window_out);
    // }
    
    // ---------------------------------------------------------------------


    
  }
}

#endif // __VSI_AUTOVEC__

#endif // __SUBSTITUTION_CC__
