#include <stdint.h>
#include <ap_int.h>

void vector_fixed_div(ap_int<8,8> * __restrict__ A, 
					ap_int<8,8> * __restrict__ B, 
					ap_int<8,8> * __restrict__ C) {
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 512; i++) {
          
		  C[i] = A[i] / B[i];  // fixed div
	} 		 
}
