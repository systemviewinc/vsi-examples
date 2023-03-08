#include "complex.h"
using namespace std;

void vector_complex_cconj(float * __restrict__ A, float * __restrict__ C) {

	complex<float> * __restrict Ai = (complex<float> * __restrict__)A; 
        complex<float> * __restrict Co = (complex<float> * __restrict__)C; 
	
        #pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		Co[i] = cconj(Ai[i]);
}
