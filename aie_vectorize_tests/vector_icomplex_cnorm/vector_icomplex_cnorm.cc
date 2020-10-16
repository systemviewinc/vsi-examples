#include "_complex.h"


void vector_icomplex_cnorm(int32_t * __restrict__ A, int32_t * __restrict__ C) {

	_complex<int32_t> * __restrict Ai = (_complex<int32_t> * __restrict__)A; 
	
        #pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C[i] = cnorm(Ai[i]);
}
