// #include "complex.h"
// using namespace std;

// void scalar_fixedcomplex_mul(int32_t * __restrict__ A, int32_t * __restrict__ B, int32_t * __restrict__ C0, int32_t * __restrict__ C1) {
// 	complex<ap_int<1,15>> * __restrict Ai = (complex<ap_int<1,15>> * __restrict__)A;
// 	complex<ap_int<1,15>> * __restrict Bi = (complex<ap_int<1,15>> * __restrict__)B;
// 	complex<ap_int<1,15>> * __restrict C0o = (complex<ap_int<1,15>> * __restrict__)C0;
// 	complex<ap_int<1,15>> * __restrict C1o = (complex<ap_int<1,15>> * __restrict__)C1;
// 	ap_int<1,15> coeff(16384);
// #pragma clang loop vectorize(disable) //interleave_count(2)
// 	for (int i = 0 ; i < 512; i++)
// 		C0o[i] = Ai[i] * Bi[i];

// #pragma clang loop vectorize(disable) //interleave_count(2)
// 	for (int i = 0 ; i < 512; i++)
// 		C1o[i] = Ai[i] * coeff;

// }

#include "complex.h"
using namespace std;

void scalar_fixedcomplex_mul(complex<ap_int<1,15>> * __restrict__ A, 
								complex<ap_int<1,15>> * __restrict__ B, 
								complex<ap_int<1,15>> * __restrict__ C0, 
								complex<ap_int<1,15>> * __restrict__ C1) {
	ap_int<1,15> coeff(16384);
#pragma clang loop vectorize(disable) //interleave_count(2)
	for (int i = 0 ; i < 512; i++)
		C0[i] = A[i] * B[i];

#pragma clang loop vectorize(disable) //interleave_count(2)
	for (int i = 0 ; i < 512; i++)
		C1[i] = A[i] * coeff;

}

