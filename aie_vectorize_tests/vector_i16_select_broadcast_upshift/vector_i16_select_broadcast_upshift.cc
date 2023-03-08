#include <stdint.h>
using namespace std;
void vector_i16_select_broadcast_upshift(int16_t * __restrict__ A, int16_t * __restrict__ B, int16_t * __restrict__ C) {
int16_t val10 = B[2] * 10;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		C[i] = A[i] * 8;
		if(A[i] > 16)
			C[i] = val10 + A[i];
	}		 
}
