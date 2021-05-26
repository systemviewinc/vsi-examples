#include "_complex.h"


void stream_cfloat(float * __restrict__ A, float * __restrict__ B,  float * __restrict__ C0, float * __restrict__ C1) {
        _complex<float> * __restrict Ai = (_complex<float> * __restrict__)A;
        _complex<float> * __restrict Bi = (_complex<float> * __restrict__)B;
        _complex<float> * __restrict C0o = (_complex<float> * __restrict__)C0;
        _complex<float> * __restrict C1o = (_complex<float> * __restrict__)C1;


        #pragma clang loop vectorize(disable)
        for (int i = 0 ; i < 64; i++)
            C0o[i] =   Ai[i] + Bi[i];

        #pragma clang loop vectorize(enable)
        for (int i = 0 ; i < 64; i++)
            C1o[i] =   Ai[i] + Bi[i];
}
