#include "complex.h"
using namespace std;
void vector_i16complex_select_broadcast(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ C) {
	complex<int16_t> * __restrict Ai = (complex<int16_t> * __restrict__)A; 
	complex<int16_t> * __restrict Co = (complex<int16_t> * __restrict__)C;
	
    complex<int16_t> one(1,1);
	complex<int16_t> three(3,3);
    #pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 32; i++){
      if(B[i] > 16)
		 Co[i] =  Ai[i] + one;
      else
		 Co[i] =  Ai[i] + three;
	}
} 