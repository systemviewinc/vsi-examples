#include "_complex.h"


void vector_complex_cconj(float * __restrict__ A, float * __restrict__ C) {

	_complex<float> * __restrict Ai = (_complex<float> * __restrict__)A; 
        _complex<float> * __restrict Co = (_complex<float> * __restrict__)C; 
	
        #pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		Co[i] = cconj(Ai[i]);
}
