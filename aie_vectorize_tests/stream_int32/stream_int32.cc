#include "_complex.h"


void stream_int32(int32_t * __restrict__ A, int32_t * __restrict__ B,  int32_t * __restrict__ C0, int32_t * __restrict__ C1) {

    	#pragma clang loop vectorize(disable)
			for (int i = 0 ; i < 64; i++)
    			C0[i] =   A[i] + B[i];

    	#pragma clang loop vectorize(enable)
			for (int i = 0 ; i < 64; i++)
    			C1[i] =   A[i] + B[i];

}
