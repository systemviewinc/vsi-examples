#ifndef _HLSFFT_H_
#define _HLSFFT_H_

//#include <hls_stream.h>
//#include <complex>
#include "complex.h"


// hard coded 1024 point FFT
#define N 1024

#if N == 256
    #define logN 8
#elif N == 1024
    #define logN 10
#else 
    #define logN 12
#endif

#pragma XMC INPORT top_in
void hlsFFT(float * __restrict top_in, float * __restrict out);
//void hlsFFT(std::complex<float> top_in[N], std::complex<float> out[N]);
//void hlsFFT(hls::stream< std::complex< float > >& top_in, hls::stream< std::complex< float > >& out);
//void bitswap(complex<float> * __restrict x,  complex<float> * __restrict y);
//void bitswap(std::complex<float> *x,  std::complex<float> *y);:W
//
//void FFT_bf(std::complex<float> *x, std::complex<float> *y);
//void FFT_bf(complex<float> * __restrict x, complex<float> * __restrict y);
void init_twiddle_table(float * __restrict sin_table, float * __restrict cos_table);
//void butterfly(unsigned short w_factor, unsigned short w_step_bits, const float *sin_table, const float *cos_table, std::complex<float> in, std::complex<float> out);
//void butterfly(unsigned short* __restrict w_factor, unsigned short * __restrict w_step_bits, const float * __restrict sin_table, const float * __restrict cos_table, complex<float>* __restrict in, complex<float>* __restrict out);
//unsigned short cir_shift_right(unsigned short in, int Nbits);
static inline void cir_shift_right_1(unsigned short* __restrict in, unsigned short* __restrict results);
static inline void cir_shift_right_2(unsigned short* __restrict in, unsigned short* __restrict results);

#endif 
