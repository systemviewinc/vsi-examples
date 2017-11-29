#include <hls_stream.h>
template<int a, int b, int c>
struct cl {
	int data;
};

struct clz {
	int data;
};

typedef cl<1,2,3> clx;
typedef clz clzx;

void foobar(hls::stream<clzx> &c) { }

void foobar1(hls::stream<clx> &c) { }
