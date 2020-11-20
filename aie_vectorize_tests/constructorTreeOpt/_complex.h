#include <stdint.h>
#define _VSI_LLVM_

template <class T>
class __attribute__((packed)) _complex {
	union {
		struct {
			T re;
			T im;
		} s;
		double d;
	} u ;

 public:
	_complex() {}
	_complex(T r, T i) {
		u.s.re = r;
		u.s.im = i;
	}
	_complex(T r) {
		u.s.re = r;
		u.s.im = 0;
	}
	_complex(double d) {
		u.d = d;
	}
	T &real()  { return u.s.re; };
	T &imag()  { return u.s.im; };
	double to_d() const { return u.d ; };
	
	void operator=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = x.u.d;
#else		
		u.s.re = x.u.s.re;
		u.s.im = x.u.s.im;
#endif
	}
	void operator+=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = __builtin_vsi_cfadd(u.d,x.u.d);
#else		
		u.s.re += x.u.s.re;
		u.s.im += x.u.s.im;
#endif
	}

	void operator-=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = __builtin_vsi_cfsub(u.d,x.u.d);
#else		
		u.s.re -= x.u.s.re;
		u.s.im -= x.u.s.im;
#endif
	}

	void operator*=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = __builtin_vsi_cfmult(u.d,x.u.d);
#else		
		u.s.re += x.u.s.re;
		u.s.im += x.u.s.im;
#endif
	}
};

template <>
class __attribute__((packed)) _complex <int32_t> {
	union {
		struct {
			int32_t re;
			int32_t im;
		} s;
		int64_t d;
	} u ;

 public:
	_complex() {}
	_complex(int32_t r, int32_t i) {
		u.s.re = r;
		u.s.im = i;
	}
	_complex(int32_t r) {
		u.s.re = r;
		u.s.im = 0;
	}
	_complex(int64_t d) {
		u.d = d;
	}
	int32_t &real() { return u.s.re; };
	int32_t &imag() { return u.s.im; };
	int64_t to_d() const { return u.d ; };
	
	void operator=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = x.u.d;
#else		
		u.s.re = x.u.s.re;
		u.s.im = x.u.s.im;
#endif
	}
	void operator+=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = __builtin_vsi_cadd(u.d,x.u.d);
#else		
		u.s.re += x.u.s.re;
		u.s.im += x.u.s.im;
#endif
	}

	void operator-=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = __builtin_vsi_csub(u.d,x.u.d);
#else		
		u.s.re -= x.u.s.re;
		u.s.im -= x.u.s.im;
#endif
	}

	void operator*=(const _complex x)  {
#ifdef _VSI_LLVM_
		u.d = __builtin_vsi_cmult(u.d,x.u.d);
#else		
		u.s.re += x.u.s.re;
		u.s.im += x.u.s.im;
#endif
	}
};

template<class T>
static inline _complex<T> operator+(_complex<T> x, _complex<T> y)  {
	_complex<T> r(x.real(),x.imag());
	r += y;
	return r;
}
template<class T>
static inline _complex<T> operator-(_complex<T> x, _complex<T> y)  {
	_complex<T> r(x.real(),x.imag());
	r -= y;
	return r;
}

template<class T>
static inline _complex<T> operator*(_complex<T> lhs, _complex<T> rhs)
{
	_complex<T> r(lhs.real(),lhs.imag());
	r *= rhs;
	return r;
}

template<class T>
static inline _complex<T> cconj(const _complex<T> x) {
#ifdef _VSI_LLVM_
	_complex<T> xconj((double)__builtin_vsi_cfconj(x.to_d()));
#else
	_complex xconj( x.real(),-1.0f*x.imag());
#endif   
	return xconj;
}

template<>
inline _complex<int32_t> cconj(const _complex<int32_t> x) {
#ifdef _VSI_LLVM_
	_complex<int32_t> xconj((int64_t)__builtin_vsi_cconj(x.to_d()));
#else
	_complex xconj( x.real(),-1.0f*x.imag());
#endif   
	return xconj;
}

template<class T>
static inline _complex<T> operator*(_complex<T> lhs, T rhs)
{
#ifdef _VSI_LLVM_
	_complex<T> prod((double)__builtin_vsi_cfmultf(lhs.to_d(),rhs));
#else   
	_complex prod(lhs.real() * rhs, lhs.imag() * rhs);
#endif   
	return prod;
}

template<>
inline _complex<int32_t> operator*(_complex<int32_t> lhs, int32_t rhs)
{
#ifdef _VSI_LLVM_
	_complex<int32_t> prod((int64_t)__builtin_vsi_cmultf(lhs.to_d(),rhs));
#else   
	_complex prod(lhs.real() * rhs, lhs.imag() * rhs);
#endif   
	return prod;
}

template<class T>
static inline T cnorm(const _complex<T> x)
{
#ifdef _VSI_LLVM_
	return (T)__builtin_vsi_cfnorm(x.to_d());
#else       
	return x.real()*x.real() + x.imag()*x.imag();
#endif	
}

template<>
inline int32_t cnorm(const _complex<int32_t> x)
{
#ifdef _VSI_LLVM_
	return (int32_t)__builtin_vsi_cnorm(x.to_d());
#else       
	return x.real()*x.real() + x.imag()*x.imag();
#endif	
}

