#include <stdint.h>
#include <ap_int.h>

void scalar_fixed_add_sub_mul(ap_int<1,15> * __restrict__ A,
							ap_int<1,15> * __restrict__ B, 
							ap_int<1,15> * __restrict__ C0, 
							ap_int<1,15> * __restrict__ C1, 
							ap_int<1,15> * __restrict__ C2) {
#pragma clang loop vectorize(disable) //interleave_count(1)
	for (int i = 0 ; i < 1024; i++) {
          C0[i] = A[i] + B[i];  // fixed add
		  C1[i] = A[i] - B[i];  // fixed sub
		  C2[i] = A[i] * B[i];  // fixed mul
	}
}
