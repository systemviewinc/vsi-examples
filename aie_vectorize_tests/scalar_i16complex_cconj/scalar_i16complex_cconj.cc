#include "complex.h"
using namespace std;
void scalar_i16complex_cconj(int16_t * __restrict__ A, int16_t * __restrict__ C) {
	complex<int16_t> * __restrict Ai = (complex<int16_t> * __restrict__)A; 
	complex<int16_t> * __restrict Co = (complex<int16_t> * __restrict__)C;

    #pragma clang loop vectorize(disable)
	for (int i = 0 ; i < 32; i++) 
    		Co[i] =  cconj(Ai[i]);
} 