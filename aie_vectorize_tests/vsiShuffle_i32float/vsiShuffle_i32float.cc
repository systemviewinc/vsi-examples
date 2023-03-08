#include <stdint.h>
using namespace std;

void vsiShuffle_i32float(float * __restrict__ A, int32_t * __restrict__ B, float * __restrict__ C0, int32_t * __restrict__ C1) {


    #pragma clang loop vectorize(enable) interleave_count(1)
    for (int i = 0; i < 64; i++) {
        C0[i] =  A[i*2] + 2.0f;
        C1[i] =  B[i*2] + 4;
    }
}
