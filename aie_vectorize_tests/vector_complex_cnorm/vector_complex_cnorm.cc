#include "complex.h"


void vector_complex_cnorm(float * __restrict__ A, float * __restrict__ C) {

	complex<float> * __restrict Ai = (complex<float> * __restrict__)A; 
	
        #pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C[i] = cnorm(Ai[i]);
}
