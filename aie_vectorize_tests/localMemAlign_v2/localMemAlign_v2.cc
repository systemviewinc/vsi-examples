#include <stdint.h>

void localMemAlign_v2(int32_t * __restrict__ A, int32_t * __restrict__ B,  int32_t * __restrict__ C0, int32_t * __restrict__ C1) {

       int32_t localMem [64];

       for (int i = 0 ; i < 64; i++)
          localMem[i] = A[i];

    	#pragma clang loop vectorize(disable)
			for (int i = 0 ; i < 64; i++)
    			C0[i] =   localMem[i] + B[i];

    	#pragma clang loop vectorize(enable)
			for (int i = 0 ; i < 64; i++)
    			C1[i] =   localMem[i] + B[i];

}
