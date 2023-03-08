#include "complex.h"
using namespace std;
void vector_complex_select_broadcast(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
	complex<float> * __restrict Ai = (complex<float> * __restrict__)A; 
	complex<float> * __restrict Co = (complex<float> * __restrict__)C;
	
    complex<float> two(2.0f,2.0f);
	complex<float> three(3.0f,3.0f);
    #pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
      if(B[i] > 32)
		 Co[i] =  Ai[i] + two ;
      else
		 Co[i] =  Ai[i] + three;
	}
} 