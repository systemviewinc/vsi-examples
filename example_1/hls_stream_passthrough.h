#ifndef  HLS_STREAM_PASSTHROUGH
#define HLS_STREAM_PASSTHROUGH

#include <hls_stream.h>

typedef struct {
	char buf[16] ;
} char16;

void test_java(hls::stream<char16> &inc, hls::stream<char16> &out);

#endif /* end of include guard: HLS_STREAM_PASSTHROUGH */
