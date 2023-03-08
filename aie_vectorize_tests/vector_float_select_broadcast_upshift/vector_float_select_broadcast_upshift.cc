#include <stdint.h>
using namespace std;
void vector_float_select_broadcast_upshift(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
float val10 = B[2] * 10.0f;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = A[i] * 8.0f;
		if(A[i] > 16.0f)
			C[i] = val10 + A[i];
	}		 
}
