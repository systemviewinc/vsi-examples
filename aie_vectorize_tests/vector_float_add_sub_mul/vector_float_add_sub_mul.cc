#include <stdint.h>

void vector_float_add_sub_mul(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {
float tmp_0;
float tmp_1;
float val10 = 2.0f;
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
          tmp_0 = A[i] + val10;  // float add
		  tmp_1 = B[i] - val10;  // float sub
		  C[i] = tmp_0 * tmp_1;  // float mul
	} 		 
}
