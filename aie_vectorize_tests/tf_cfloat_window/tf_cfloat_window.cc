#include "complex.h"
using namespace std;

void tf_cfloat_window(complex<float> * __restrict__ A, complex<float> * __restrict__ B, complex<float> * __restrict__ C0, complex<float> * __restrict__ C1) {


        #pragma clang loop vectorize(disable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C0[i] =  A[i] + B[i];

        #pragma clang loop vectorize(enable) //interleave_count(1)
        for (int i = 0 ; i < 64; i++)
            C1[i] =  A[i] + B[i];
}
