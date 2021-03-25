#include "_complex.h"

void vector_complex_select_broadcast(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
	_complex<float> * __restrict Ai = (_complex<float> * __restrict__)A; 
	_complex<float> * __restrict Co = (_complex<float> * __restrict__)C;
	
    _complex<float> two(2.0f,2.0f);
	_complex<float> three(3.0f,3.0f);
    #pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
      if(B[i] > 32)
		 Co[i] =  Ai[i] + two ;
      else
		 Co[i] =  Ai[i] + three;
	}
} 