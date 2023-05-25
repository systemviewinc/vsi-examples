#include <stdint.h>
#include <ap_int.h>

void scalar_fixed_add_sub_mul(int * __restrict__ A, int * __restrict__ B, 
		int * __restrict__ C0, int * __restrict__ C1, int * __restrict__ C2) {


ap_int<1,15> * Af = (ap_int<1,15>*)A;
ap_int<1,15> * Bf = (ap_int<1,15>*)B;
ap_int<1,15> * C0f = (ap_int<1,15>*)C0;
ap_int<1,15> * C1f = (ap_int<1,15>*)C1;
ap_int<1,15> * C2f = (ap_int<1,15>*)C2;
#pragma clang loop vectorize(disable) //interleave_count(1)
	for (int i = 0 ; i < 1024; i++) {
          C0f[i] = Af[i] + Bf[i];  // fixed add
		  C1f[i] = Af[i] - Bf[i];  // fixed sub
		  C2f[i] = Af[i] * Bf[i];  // fixed mul
	}
}
