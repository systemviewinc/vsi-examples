#include <string.h>
#include <hls_stream.h>
#include <vsi_device.h>
#include <unistd.h>

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

int count = 0;

void mem_test(hls::stream<int> &data_in, vsi::device &mem) {
	int offset = 0;
	printf("entered func\n");
	data_in.wait_if_empty();
	while(!data_in.empty()) {
		printf("loop enter\n");
		int i = data_in.read();
		printf("data read %s\n", &i);
		mem.pwrite(&i, sizeof(i), offset);
		printf("data write\n");
		int out = 0;
		mem.pread(&out, sizeof(i), offset);
		offset+= sizeof(int);
		if (i != out) {
			printf("incorrect value read %x\n", out);
		}
		if (count % 1000)
			printf("wrote %d times\ncurrent offset: %d\n", count, offset);
		count ++;
	}
}

void mem_test_write(hls::stream<int> &data_in, hls::stream<int> &data_out, vsi::device &mem) {
	int offset = 0;
	printf("entered func\n");
	data_in.wait_if_empty();
	while(!data_in.empty()) {
		printf("loop enter\n");
		int i = data_in.read();
		printf("data read %s\n", &i);
		mem.pwrite(&i, sizeof(i), offset);
		printf("data write\n");
		int out = 0;
		mem.pread(&out, sizeof(i), offset);
		data_out.write(out);
		offset+= sizeof(int);
		if (i != out) {
			printf("incorrect value read %x\n", out);
		}
		if (count % 1000)
			printf("wrote %d times\ncurrent offset: %d\n", count, offset);
		count ++;
	}
}

void write_test(char str[265]) {
	strcpy(str, "TEST STRING");
}

void void_test() {
	printf("void stuff\n");
}

// non synthesizable
#ifndef __VSI_HLS_SYN__
void write_4k(int outd[1024])
{
	static int first = 0;
	if (!first) {
		first = 1;
		printf("Sending data\n");
	} else {
		printf("going to sleep\n");
		while(1) sleep(1);
	}
	for (int i = 0 ; i < 1024; i++) outd[i] = i;	
}

void read_4k(int ind[1024])
{
	printf("Got Data\n");
	sleep(1);
	exit(0);
}
#endif
