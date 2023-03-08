#include "complex.h"
using namespace std;

void vector_icomplex_select_broadcast(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C) {
	complex<int32_t> * __restrict Ai = (complex<int32_t> * __restrict__)A; 
	complex<int32_t> * __restrict Co = (complex<int32_t> * __restrict__)C;
	
    complex<int32_t> one(1,1);
    #pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++){
      if(B[i] > 32)
		 Co[i] =  Ai[i] + one;
      else
		Co[i] =  Ai[i];
	}
} 