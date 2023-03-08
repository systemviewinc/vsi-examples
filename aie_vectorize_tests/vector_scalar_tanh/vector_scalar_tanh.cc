#include <stdint.h>
#include "math.h"
using namespace std;
void vector_scalar_tanh(float * __restrict__ A, float * __restrict__ B, float * __restrict__ C) {

    float tmp = B[9];//tanhf(B[9]);

    #pragma clang loop vectorize(enable) 
	for (int i = 0 ; i < 64; i++) {
        C[i] = tanhf(A[i]) + tmp;
	}		 
}
