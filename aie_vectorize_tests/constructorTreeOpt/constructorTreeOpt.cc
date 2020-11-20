#include "_complex.h"
#include <math.h>

void constructorTreeOpt(float * __restrict__ A, float * __restrict__ C) {

	_complex<float> * __restrict Ai = (_complex<float> * __restrict__)A;
	_complex<float> * __restrict Co = (_complex<float> * __restrict__)C;
    _complex<float> Bi;
	#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++){
		    	Bi.real() =  sinf(i);
	        Bi.imag() =  cosf(i);
	        Co[i] = Bi*Ai[i];
	}
}
