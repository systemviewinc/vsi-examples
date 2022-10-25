// RTX Proprietary, except for System View company, who agreed to keep internal to System View
// Optimization of STAP through increased vectorization by code changes
#ifndef __ALL_STAP_CC__
#define __ALL_STAP_CC__

#include "kernels.h"

#ifdef __VSI_AUTOVEC__
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "_complex.h"


void cholesky_factorization(_complex<float> * __restrict__ cholesky_factors,
			    _complex<float> * __restrict__ cholesky_factors_c,
			    _complex<float> * __restrict__ covariance) {
	int i, j, k, chol_index; 
	float Rkk_inv, Rkk_inv_sqrt;
   
	/*
	 * cholesky_factors is a working buffer used to factorize the
	 * covariance matrices in-place.  We copy the covariance matrices
	 * into cholesky_factors and give cholesky_factors the convenient
	 * name R for a more succinct inner loop below.
	 */
	memcpy(cholesky_factors, covariance, sizeof(_complex<float>)*N_CHAN*TDOF*N_CHAN*TDOF);
   
	/*
	 * The following Cholesky factorization notation is based
	 * upon the presentation in "Numerical Linear Algebra" by
	 * Trefethen and Bau, SIAM, 1997.
	 */
   
	for (k = 0; k < N_CHAN*TDOF; ++k) {
		_complex<float> *ck = &cholesky_factors[k*N_CHAN*TDOF];
		/*
		 * Hermitian positive definite matrices are assumed, but
		 * for safety we check that the diagonal is always positive.
		 */
		/* assert(cholesky_factors[k*N_CHAN*TDOF+k].re > 0); */
	   
		/* Diagonal entries are real-valued. */
		Rkk_inv = 1.0f / ck[k].real();
		Rkk_inv_sqrt = sqrtf(Rkk_inv);
	   
		for (j = k+1; j < N_CHAN*TDOF; ++j) {
			const _complex<float> Rkj_conj = cconj(ck[j]);
#pragma clang loop vectorize(enable)
			for (i = 0; i < N_CHAN*TDOF; ++i) {
				_complex<float> tmp(0.0f,0.0f);
				const _complex<float> Rki_Rkj_conj = cholesky_factors_c[k*N_CHAN*TDOF+i] * Rkj_conj;
			   
				if(i>=j)
					tmp = Rki_Rkj_conj * Rkk_inv;
				chol_index = j*N_CHAN*TDOF+i;
				cholesky_factors[chol_index] -= tmp;
			   
			}
		} 
	   
#pragma clang loop vectorize(enable)
		for (i = 0; i < N_CHAN*TDOF; ++i) {
			chol_index = k*N_CHAN*TDOF+i;
			float tmp = Rkk_inv_sqrt;
			if(i < k)
				tmp = 1.0f;
			cholesky_factors[chol_index] = cholesky_factors[chol_index] * tmp;
		   
		}
	}
	/*
	 * Copy the conjugate of the upper triangular portion of cholesky_factors
	 * into the lower triangular portion. This is not required
	 * for correctness, but can help with testing and validation
	 * (e.g., correctness metrics calculated over all elements
	 * will not be "diluted" by trivially correct zeros in the
	 * lower diagonal region).
	 */
	_complex<float> cc[N_CHAN*TDOF*N_CHAN*TDOF];
   
	for (i = 0; i < N_CHAN*TDOF*N_CHAN*TDOF; ++i) {
		cc[i] = cholesky_factors[i];
	}
   
	for (i = 0; i < N_CHAN*TDOF; ++i) {
#pragma clang loop vectorize(enable)
		for (j = 0; j < N_CHAN*TDOF; ++j) {
			chol_index = j*N_CHAN*TDOF+i;
			const _complex<float> y = cc[j*N_CHAN*TDOF+i];
			const _complex<float> x = conj(cc[i*N_CHAN*TDOF+j]);
			_complex<float> rv;
			if (j< i+1){
				rv = y;
			} else {
				rv = x;
			}
			cholesky_factors[j*N_CHAN*TDOF+i] = rv;
		}
	}
} 


#else

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "cholesky.cpp"  // this is the name of the VSI output generated vectorized source file

// From Steve:
// INTERNAL_ITERATION_NUM = dops * blocks = 512*2=1024
// However, if using multiple AIE and wanting latency, divide that number by # of AIE used
// For example with 128 AIE used for STAP, we would change 1024 to 8
#define INTERNAL_ITERATION_NUM 1  // 1024 full value but divided by # AIE so latency shown in sim is correct.

void cholesky_top(
		  // just running the cholesky function ( for partitioning)
		  input_stream_float  * cov_stream_in,
		  // outputs from this kernel
		  output_window_float * final_out_data_window_out) {
	// int latency = get_cycles();
	float gamma_buf;
   
	for ( int internal_iteration = 0 ; internal_iteration < INTERNAL_ITERATION_NUM; internal_iteration++) {
		for (int i = 0; i < ( sizeof(covariance_buffer)/ sizeof(float) ); i++ ) {
			covariance_buffer[i] = readincr(cov_stream_in);
		}

		struct class__complex* covariance_complex = (struct class__complex*)covariance_buffer;
		struct class__complex* final_data_complex_out = (struct class__complex*)final_data_out_buffer;

		// calling the vectorized cholesky_factorization
		// cholesky_factorization(cholesky_factor_complex, covariance_complex);
		cholesky_factorization(final_data_complex_out,final_data_complex_out, covariance_complex);
      
		window_acquire(final_out_data_window_out);
		//		// Send final data out
		for (int i = 0; i < ( sizeof(final_data_out_buffer)/ sizeof(float) ); i++ ) {
			window_writeincr(final_out_data_window_out, final_data_out_buffer[i]);
		}
		window_release(final_out_data_window_out);

		// ---------------------------------------------------------------------
	}
	// latency = get_cycles() - latency;
}

#endif
#endif
