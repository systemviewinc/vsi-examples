
#ifndef __ALL_STAP_CC__
#define __ALL_STAP_CC__

#include "kernels.h"
#define N_CHAN (4)
#define TDOF (3)
#define N_STEERING (2)
#define SV_SIZE (24) // each Steering vector size is 24 floats= 12 cfloats = 96Bytes

#ifdef __VSI_AUTOVEC__
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "_complex.h"

#define N_CHAN (4)
#define N_RANGE (512)
#define TRAINING_BLOCK_SIZE (32)
#define N_BLOCKS (N_RANGE/TRAINING_BLOCK_SIZE)
#define N_PULSES (128)
#define N_DOP (256)
#define TDOF (3)


void stap_compute_covariance_estimate(
	_complex<float> * __restrict__ covariance,
	_complex<float> * __restrict__ snapshot )
{
    int cell;
    const size_t num_cov_elements = (TDOF*N_CHAN) * (TDOF*N_CHAN) * N_DOP * N_BLOCKS;

    /*
     * It is assumed for simplicity that the training block
     * size evenly divides the range swath.
     */
    /*assert(N_RANGE % TRAINING_BLOCK_SIZE == 0); */


    int i, j, k, conv_index;

    for (k = 0; k < TRAINING_BLOCK_SIZE; k++) {
	    /* Outer products are accumulated over a full block. */
		for (i = 0; i < N_CHAN*TDOF; ++i)
		{
			/* Exploit conjugate symmetry by only accumulating along
			 * the diagonal and below. */
			#pragma clang loop vectorize(disable)
			for (j = 0; /*j <N_CHAN*TDOF*/j <= i; ++j)
				{
					/*const*/ _complex<float> x = snapshot[k*N_CHAN*TDOF+i] * cconj(snapshot[k*N_CHAN*TDOF+j]);
					conv_index = i*N_CHAN*TDOF + j;
					covariance[conv_index] += (x);
				}
		}
    }

    /*
     * The covariance matrices are conjugate symmetric, so
     * we copy the conjugate of the lower triangular portion
     * into the upper triangular portion.
     */
    for (i = 0; i < N_CHAN*TDOF; ++i)
            {
		    for (j = i+1; j < N_CHAN*TDOF; ++j)
			    {
				    const _complex<float> x = covariance[j*N_CHAN*TDOF+i];
				    conv_index = i*N_CHAN*TDOF + j;
				    covariance[conv_index]= cconj(x);
				    // covariance[conv_index].im = -1.0f * x.im;
			    }
            }

    /*
     * Normalize the covariance matrices by dividing by the
     * number of training samples.
     */
	float sample_norm = (1.0f/TRAINING_BLOCK_SIZE);
    for (i = 0; i < N_CHAN*TDOF; ++i)
            {
		#pragma clang loop vectorize(enable)
		    for (j = 0; j < N_CHAN*TDOF; ++j)
			    {
				    conv_index = i*N_CHAN*TDOF + j;

				    covariance[conv_index] = covariance[conv_index] * sample_norm;
				    // covariance[conv_index].im *= (1.0f/TRAINING_BLOCK_SIZE);
			    }
            }
}
/*
 * Kernel 2: Weight Generation (Linear System Solver)
 */

void cholesky_factorization(_complex<float> * __restrict__ cholesky_factors,
			     _complex<float> * __restrict__ covariance)

{
    int i, j, k, chol_index;
    float Rkk_inv, Rkk_inv_sqrt;

    /*
     * cholesky_factors is a working buffer used to factorize the
     * covariance matrices in-place.  We copy the covariance matrices
     * into cholesky_factors and give cholesky_factors the convenient
     * name R for a more succinct inner loop below.
     */
     memcpy(cholesky_factors, covariance,
	    sizeof(_complex<float>)*N_CHAN*TDOF*N_CHAN*TDOF);
    /*
     * The following Cholesky factorization notation is based
     * upon the presentation in "Numerical Linear Algebra" by
     * Trefethen and Bau, SIAM, 1997.
     */
    for (k = 0; k < N_CHAN*TDOF; ++k) {
	    /*
	     * Hermitian positive definite matrices are assumed, but
	     * for safety we check that the diagonal is always positive.
	     */
	    /* assert(cholesky_factors[k*N_CHAN*TDOF+k].re > 0); */

	    /* Diagonal entries are real-valued. */
	    Rkk_inv = 1.0f / cholesky_factors[k*N_CHAN*TDOF+k].real();
	    Rkk_inv_sqrt = sqrtf(Rkk_inv);

	    for (j = k+1; j < N_CHAN*TDOF; ++j) {
		    const _complex<float> Rkj_conj = cconj(cholesky_factors[k*N_CHAN*TDOF+j]);
		    for (i = 0; i < N_CHAN*TDOF; ++i) {
					_complex<float> zero(0.0f,0.0f);
					_complex<float> tmp;

			    const _complex<float> Rki_Rkj_conj = cholesky_factors[k*N_CHAN*TDOF+i] * Rkj_conj;
					tmp = Rki_Rkj_conj * Rkk_inv;
					if(i<j)
							tmp = zero;
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
    for (i = 0; i < N_CHAN*TDOF; ++i) {
	    for (j = i+1; j < N_CHAN*TDOF; ++j) {
		    chol_index = j*N_CHAN*TDOF+i;
		    const _complex<float> x = cholesky_factors[i*N_CHAN*TDOF+j];
		    cholesky_factors[chol_index] = cconj(x);
		    // cholesky_factors[chol_index].im = -1.0f * x.im;
	    }
    }
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
void forward_and_back_substitution(
				   _complex<float> * __restrict__ adaptive_weights,
				    _complex<float> * __restrict__ cholesky_factors,
					_complex<float> * __restrict__ steering_vectors,
				   float * __restrict__ gamma)
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
{
	/*
	 * We are solving the system R*Rx = b where upper triangular matrix R
	 * is the result of Cholesky factorization.  To do so, we first apply
	 * forward substitution to solve R*y = b for y and then apply back
	 * substitution to solve Rx = y for x.  In this case, b and x correspond
	 * to the steering vectors and adaptive weights, respectively.
	 */

	int i, j, k;
	_complex<float> accum;

	/* First apply forward substitution */
	for (i = 0; i < N_CHAN*TDOF; ++i) {
		const float Rii_inv = 1.0f / cholesky_factors[i*N_CHAN*TDOF+i].real();
		accum.real() = 0.0f;
		accum.imag() = 0.0f;
		for (j = 0; j < N_CHAN*TDOF;  ++j) {
			/*
			 * Use the conjugate of the upper triangular entries
			 * of cholesky_factors as the lower triangular entries.
			 */
			_complex<float> zero(0.0f, 0.0f);

			_complex<float> prod = zero;
			if(j<i)
				prod = cconj(cholesky_factors[j*N_CHAN*TDOF+i]) * adaptive_weights[j];
			
			accum += prod;
			// accum.im += prod.im;
		}
		adaptive_weights[i] = (steering_vectors[i] - accum) * Rii_inv;
		// adaptive_weights[i].im = (steering_vectors[i].im - accum.im) * Rii_inv;
	}

	/* And now apply back substitution */
	for (j = N_CHAN*TDOF-1; j >= 0; --j) {
		const float Rjj_inv = 1.0f / cholesky_factors[j*N_CHAN*TDOF+j].real();
		accum.real() = 0.0f;
		accum.imag() = 0.0f;
		for (k = j+1; k < N_CHAN*TDOF; ++k) {
			const _complex<float> prod = cholesky_factors[j*N_CHAN*TDOF+k] * adaptive_weights[k];
			accum += prod;
			// accum.im += prod.im;
		}
		adaptive_weights[j] = (adaptive_weights[j] - accum) * Rjj_inv;
		// adaptive_weights[j].im = (adaptive_weights[j].im - accum.im) * Rjj_inv;
	}
	/*
     * calculating the gama in forward_and_back_substitution
	 * to reduce the number of AIEs
     */

    _complex<float> accum_gamma;

    accum_gamma.real() = 0.0f;
	accum_gamma.imag() = 0.0f;
    for (i = 0; i < N_CHAN*TDOF; ++i)
	    {
		    const _complex<float> prod = 
					       cconj(adaptive_weights[i]) *
					       steering_vectors[i];
		    accum_gamma += prod;
		    // accum_gamma.im += prod.im;
	    }

    /*
     * In exact arithmetic, accum should be a real positive
     * scalar and thus the imaginary component should be zero.
     * However, with limited precision that may not be the case,
     * so we take the magnitude of accum.  Also, gamma is a
     * normalization scalar and thus we take the inverse of
     * the computed inner product, w*v.
     */
    *gamma = sqrtf(accum_gamma.real()*accum_gamma.real() + accum_gamma.imag()*accum_gamma.imag());
    if (*gamma > 0)
	    {
		    *gamma = 1.0f / *gamma;
	    }
    else
	    {
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
    int length = TDOF*N_CHAN;
    // for the first set of 18 kernels (all STAP kernels) block is 0 , for the next set of 18 kernels the block should become 1 and so on till end of all 16 block: block = 0; block < N_BLOCKS; ++block
    int block = 0; 
    _complex<float> accum(0.0f,0.0f);
    _complex<float> zero(0.0f,0.0f);
    int i,cell, ci;
    const int first_cell = block*TRAINING_BLOCK_SIZE;
    const int last_cell = (block+1)*TRAINING_BLOCK_SIZE-1;
    for (cell = first_cell, ci = 0; cell <= last_cell; ++cell, ci++) {
        accum = zero;
        for (i = 0; i < length; ++i)
        {
            const _complex<float> prod = 
                cconj(lhs[i])* rhs[ci*length +i];
            accum += prod;
        }
    accum =accum * (*gamma); 
    outvalue[ci] = accum;
    }
}

#else

#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
#include "all_stap.cpp"

#define INTERNAL_ITERATION_NUM 1 //256 original value


void stap_kernel
(
    // snapshot input from RDMA
    input_stream_float  * snapshot_stream_in,
    // steering_vectors input from RDMA
	input_window_float  * steering_vectors_window_in,
	// outputs from this kernel
    output_window_float * final_out_data_window_out
) {
	float gamma_buf;
	float *steering_vec_ptr = (float *)steering_vectors_window_in->ptr;

	for (int i = 0; i < ( sizeof(steering_vector_in_buffer)/ sizeof(float) ); i++ ) {	
		steering_vector_in_buffer[i] = steering_vec_ptr[i];
	}

	for ( int internal_iteration = 0 ; internal_iteration < INTERNAL_ITERATION_NUM; internal_iteration++) {
		// printf("covariance_kernel FUNCTION: internal_iteration of 16*256 = %d \n", internal_iteration);
		for (int i = 0; i < ( sizeof(snapshot_in_buffer)/ sizeof(float) ); i++ ) {
			snapshot_in_buffer[i] = readincr(snapshot_stream_in);
		}
		for (int i = 0; i < ( sizeof(covariance_buffer)/ sizeof(float) ); i++ ) {
			covariance_buffer[i] = 0.0f;
		}

		struct class__complex* covariance_complex = (struct class__complex*)covariance_buffer;
		struct class__complex* snapshot_complex_in = (struct class__complex*)snapshot_in_buffer;
		struct class__complex* cholesky_factor_complex = (struct class__complex*)cholesky_factor_buffer;
		// struct class__complex* covariance_complex = (struct class__complex*)covariance_in_buffer;

		// calling the vectorized stap_compute_covariance_estimate
		stap_compute_covariance_estimate(
					covariance_complex, snapshot_complex_in);
		// calling the vectorized cholesky_factorization
		cholesky_factorization(
					cholesky_factor_complex, covariance_complex);
		for (int sv=0; sv<N_STEERING; sv++ ) {
			window_acquire(final_out_data_window_out);
			// ---------------------------------------------------------------------
			struct class__complex* final_data_complex_out = (struct class__complex*)final_data_out_buffer;
			struct class__complex* adaptive_weights_complex_out = (struct class__complex*)adaptive_weights_out_buffer;
			// struct class__complex* cholesky_factor_complex = (struct class__complex*)cholesky_factor_buffer;
			struct class__complex* steering_vectors_complex_in = (struct class__complex*)(&steering_vector_in_buffer[sv*SV_SIZE]);
			// struct class__complex* snapshot_complex_in = (struct class__complex*)snapshot_in_buffer;
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
		}
		
		
		
		// 
		// ---------------------------------------------------------------------


		
	}
}

#endif

#endif