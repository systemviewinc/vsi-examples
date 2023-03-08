#include "complex.h"
#include <math.h>
using namespace std;
void constructorTreeOpt(float * __restrict__ A, float * __restrict__ C) {

	complex<float> * __restrict Ai = (complex<float> * __restrict__)A;
	complex<float> * __restrict Co = (complex<float> * __restrict__)C;
    complex<float> Bi;
	#pragma clang loop vectorize(enable) interleave_count(2)
	for (int i = 0 ; i < 64; i++){
		    	Bi.real() =  sinf(i);
	        Bi.imag() =  cosf(i);
	        Co[i] = Bi*Ai[i];
	}
}
