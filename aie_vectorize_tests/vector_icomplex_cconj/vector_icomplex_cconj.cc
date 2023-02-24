#include "complex.h"


void vector_icomplex_cconj(int32_t * __restrict__ A, int32_t * __restrict__ C) {

	complex<int32_t> * __restrict Ai = (complex<int32_t> * __restrict__)A; 
        complex<int32_t> * __restrict Co = (complex<int32_t> * __restrict__)C; 
	
        #pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		Co[i] = cconj(Ai[i]);
}
