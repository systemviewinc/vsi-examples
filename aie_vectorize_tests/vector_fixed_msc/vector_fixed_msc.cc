#include <stdint.h>
#include <ap_int.h>

void vector_fixed_msc(ap_int<1,15> * __restrict__ A, 
					ap_int<1,15> * __restrict__ B, 
					ap_int<1,15> * __restrict__ C) {
ap_int<1,15> val(16384);
#pragma clang loop vectorize(enable) //interleave_count(1)
	for (int i = 0 ; i < 64; i++) {
		  C[i] = A[i] - B[i] * val;  // fixed msc
	} 		 
}
