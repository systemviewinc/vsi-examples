#include "complex.h"
using namespace std;
void complexReduction(float * __restrict__ A, float * __restrict__ C) {

	complex<float> * __restrict Ai = (complex<float> * __restrict__)A; 
	complex<float> * __restrict Co = (complex<float> * __restrict__)C;

    complex<float> accum; 
    accum.real() = 0.0f;
    accum.imag() = 0.0f;
    #pragma clang loop vectorize(enable)            
    for (int i = 0; i < 64; ++i)
    {
        accum.real() += Ai[i].real();
        accum.imag() += Ai[i].imag();
    }

    Co[0] = accum;
}