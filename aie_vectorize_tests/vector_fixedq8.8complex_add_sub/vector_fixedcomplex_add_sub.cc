#include "complex.h"
using namespace std;

void vector_fixedcomplex_add_sub(complex<ap_int<8,8>> * __restrict__ A, 
								complex<ap_int<8,8>> * __restrict__ B, 
								complex<ap_int<8,8>> * __restrict__ C0, 
								complex<ap_int<8,8>> * __restrict__ C1) {

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C0[i] = A[i] + B[i];

#pragma clang loop vectorize(enable) //interleave_count(2)
	for (int i = 0 ; i < 64; i++)
		C1[i] = A[i] - B[i];

}
