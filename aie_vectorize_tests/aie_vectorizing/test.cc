#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "complex.h"
using namespace std;

#define TRAINING_BLOCK_SIZE (16)
#define N_RANGE (8)
void top_func(complex<float> * __restrict__ a, float *__restrict__ b, complex<float> * __restrict__ result){

    int i,k;
    complex<float> accum(0.0f,0.0f);

    for (i = 0; i < TRAINING_BLOCK_SIZE; i++)
    {
        accum = {0, 0};
        for (k = 0; k < N_RANGE; k++)
        {
            const complex<float> prod =
                conj(a[k+i*N_RANGE]) * sinf(b[k+i*N_RANGE]);
            accum += prod;
        }
        result[i] = accum;
    }
}