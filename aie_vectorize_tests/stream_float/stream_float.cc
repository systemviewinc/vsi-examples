#include <stdint.h>


void stream_float(float * __restrict__ A, float * __restrict__ B,  float * __restrict__ C0, float * __restrict__ C1) {

    	#pragma clang loop vectorize(disable)
			for (int i = 0 ; i < 64; i++)
    			C0[i] =   A[i] + B[i];

    	#pragma clang loop vectorize(enable)
			for (int i = 0 ; i < 64; i++)
    			C1[i] =   A[i] + B[i];

}
