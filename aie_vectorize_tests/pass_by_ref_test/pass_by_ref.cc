#include "complex.h"
using namespace std;
void function_call(complex<float> * __restrict__ A, complex<float> * __restrict__ C, float *gamma) {
    for (int i = 0 ; i < 64; i++)
		C[i] = A[i] * (*gamma);
}

void pass_by_reference(complex<float> * __restrict__ A, complex<float> * __restrict__ C)
 {
     float gamma = 10.0f;
     complex<float> Ai[64];
     complex<float> Ci[64];


	for (int i = 0 ; i < 64; i++) {
		Ai[i] = A[i];
    
  }

    function_call(Ai, Ci, &gamma);

    for (int i = 0 ; i < 64; i++)
		C[i] = Ci[i];


}
