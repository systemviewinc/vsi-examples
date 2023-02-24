#include "complex.h"

void scalar_i16complex_cnorm(int16_t * __restrict__ A, int16_t * __restrict__ C) {
	complex<int16_t> * __restrict Ai = (complex<int16_t> * __restrict__)A; 

    #pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 32; i++) 
    		C[i] =  cnorm(Ai[i]);
} 