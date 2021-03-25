#include "_complex.h"

void vector_i16complex_cconj(int16_t * __restrict__ A, int16_t * __restrict__ C) {
	_complex<int16_t> * __restrict Ai = (_complex<int16_t> * __restrict__)A; 
	_complex<int16_t> * __restrict Co = (_complex<int16_t> * __restrict__)C;

    #pragma clang loop vectorize(enable)
	for (int i = 0 ; i < 64; i++) 
    		Co[i] =  cconj(Ai[i]);
} 