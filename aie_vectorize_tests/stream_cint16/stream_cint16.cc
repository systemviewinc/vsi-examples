#include "complex.h"


void stream_cint16(int16_t * __restrict__ A, int16_t * __restrict__ B,  int16_t * __restrict__ C0, int16_t * __restrict__ C1) {
    complex<int16_t> * __restrict Ai = (complex<int16_t> * __restrict__)A;
    complex<int16_t> * __restrict Bi = (complex<int16_t> * __restrict__)B;
    complex<int16_t> * __restrict C0o = (complex<int16_t> * __restrict__)C0;



    #pragma clang loop vectorize(enable)
    for (int i = 0 ; i < 64; i++)
        C0o[i] =   Ai[i] + Bi[i];

    #pragma clang loop vectorize(enable)
    for (int i = 0 ; i < 64; i++)
        C1[i] =   A[i] + B[i];
}
