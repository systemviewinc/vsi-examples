#include "complex.h"

void complexReduction(float * __restrict__ A, float * __restrict__ C) {

	complex<float> * __restrict Ai = (complex<float> * __restrict__)A; 
	complex<float> * __restrict Co = (complex<float> * __restrict__)C;

    complex<float> accum(0.0f,0.0f);
    #pragma clang loop vectorize(enable)            
    for (int i = 0; i < 8; ++i)
    {
        accum += Ai[i];
    }

    Co[0] = accum;
}