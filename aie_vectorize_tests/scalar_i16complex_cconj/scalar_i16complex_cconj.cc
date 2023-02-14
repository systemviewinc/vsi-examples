#include "_complex.h"

void scalar_i16complex_cconj(int16_t * __restrict__ A, int16_t * __restrict__ C) {
	_complex<int16_t> * __restrict Ai = (_complex<int16_t> * __restrict__)A; 
	_complex<int16_t> * __restrict Co = (_complex<int16_t> * __restrict__)C;

    #pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 32; i++) 
    		Co[i] =  cconj(Ai[i]);
} 