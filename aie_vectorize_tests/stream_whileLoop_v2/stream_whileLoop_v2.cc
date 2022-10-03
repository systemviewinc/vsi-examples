#include "_complex.h"


static int32_t coeff_0[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
static float coeff_1[16] =  {2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f,17.0f};

void stream_whileLoop_v2(int32_t * __restrict__ A, float * __restrict__ B,
		   int32_t * __restrict__ C0, float * __restrict__ C1) {
				 
   _complex<int32_t> * __restrict Ai = (_complex<int32_t> * __restrict__)A;
   _complex<float> * __restrict Bi = (_complex<float> * __restrict__)B;
   _complex<int32_t> * __restrict C0o = (_complex<int32_t> * __restrict__)C0;
   _complex<float> * __restrict C1o = (_complex<float> * __restrict__)C1;

   _complex<int32_t> * __restrict complex_coeff_0 = (_complex<int32_t> * __restrict__)coeff_0;
   _complex<float> * __restrict complex_coeff_1 = (_complex<float> * __restrict__)coeff_1;

	int counter = 0;
	while(1) {
    #pragma clang loop vectorize(enable)
		for (int i=0 ; i < 8; i++) {
			*C0o++ = (*Ai++ * complex_coeff_0[i]) ;
			*C1o++ = (*Bi++ * complex_coeff_1[i]);
		}
		counter++;
		if (counter == 16)
			break;
	}
}
