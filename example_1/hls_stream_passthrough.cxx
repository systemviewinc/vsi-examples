#include <unistd.h>
#include "hls_stream_passthrough.h"


void test_java(hls::stream<char16> &inc, hls::stream<char16> &out) {
	while(1) {
		// printf("loops start... waiting\n");
		usleep(100);
		char16 buf;
		strcpy(buf.buf, "DEADBEEF1234567");
		out.write(buf);
		// printf("%s\n", buf.buf);
		char16 buf2 = inc.read();
		// printf("%s\n", buf2.buf);
	}
}
