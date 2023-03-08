#include <stdint.h>
using namespace std;

void stream_interleave2(int32_t * __restrict__ A, float * __restrict__ B,  int32_t * __restrict__ C0, float * __restrict__ C1) {

    	#pragma clang loop vectorize(enable) interleave_count(2)
			for (int i = 0 ; i < 64; i++)
    			C0[i] =   A[i] * 2;

    	#pragma clang loop vectorize(enable) interleave_count(2)
			for (int i = 0 ; i < 64; i++)
    			C1[i] =   B[i] * 2.0f;

}
