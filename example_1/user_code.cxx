#include <string.h>
#include <hls_stream.h>

char s[] = "Echo.";
int s_len = sizeof(s) -1;

void process_tcp1(
		char in1[256],
		char out1[256]){
	for(int i = 0; i < 256; i++) {
		if (i < s_len ) out1[i] = s[i];
		else out1[i] = in1[i-s_len];
	}
}

void func1(int a[16], hls::stream<int> &b, hls::stream<int> &r) {
	for(int i = 0; i < 16; i++ ) {
		b.write(a[i]);
	}
	int j = r.read();
}

void func2(hls::stream<int> &a, hls::stream<int> &r) {
	int _r = 0;
	while(!a.empty()) {
		_r += a.read();
	}
	r.write(_r);
}
