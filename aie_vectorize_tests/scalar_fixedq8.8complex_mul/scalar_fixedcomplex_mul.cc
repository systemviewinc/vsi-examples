#include "complex.h"
using namespace std;

void scalar_fixedcomplex_mul(complex<ap_int<8,8>> * __restrict__ A, 
								complex<ap_int<8,8>> * __restrict__ B, 
								complex<ap_int<8,8>> * __restrict__ C0, 
								complex<ap_int<8,8>> * __restrict__ C1) {
	ap_int<8,8> coeff(512);
#pragma clang loop vectorize(disable) //interleave_count(2)
	for (int i = 0 ; i < 512; i++)
		C0[i] = A[i] * B[i];

#pragma clang loop vectorize(disable) //interleave_count(2)
	for (int i = 0 ; i < 512; i++)
		C1[i] = A[i] * coeff;

}
