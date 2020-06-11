#include <stdint.h>

typedef struct  { float re, im; } _complex;

/* complex conjugate */
static inline _complex cconj(_complex x)
{
	_complex xconj = x;
	xconj.im *= -1.0f;
	return xconj;
}

/* complex multiplication */
static inline _complex cmult(_complex lhs, _complex rhs)
{
	_complex prod;
	prod.re = lhs.re * rhs.re - lhs.im * rhs.im;
	prod.im = lhs.re * rhs.im + lhs.im * rhs.re;
	return prod;
}

void test_complex (_complex * __restrict__ A,
		   _complex * __restrict__ B,
		   _complex * __restrict__ C) {
	for (int i = 0 ; i < 1024; i++) {
		C[i] = cmult(A[i],cconj(B[i]));
	}
}

void test_complex_top (int32_t * __restrict__ A,
		       int32_t * __restrict__ B,
		       int32_t * __restrict__ C) {
	
	test_complex((_complex * __restrict__) A,
		     (_complex * __restrict__) B,
		     (_complex * __restrict__) C);
}
