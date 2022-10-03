#include "_complex.h"


static int32_t coeff_0[8] =  {1,2,3,4,5,6,7,8};
static float coeff_1[8]   =  {2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f};

void stream_whileLoop(int32_t * __restrict__ A, float * __restrict__ B,
		   int32_t * __restrict__ C0, float * __restrict__ C1) {

		 int counter = 0;
		 while(1) {
            #pragma clang loop vectorize(enable)
		        for (int i=0 ; i < 8; i++) {
			               *C0++ = (*A++ * coeff_0[i]);
			               *C1++ = (*B++ * coeff_1[i]);
		        }
			counter++;
			if (counter == 16)
				break;
	       }
}
