#include <string.h>

char s[] = "Echo.";
int s_len = sizeof(s) -1;

int process_tcp1(
		char in1[256],
		char out1[256]){
	for(int i = 0; i < 256; i++) {
		if (i < s_len ) out1[i] = s[i];
		else out1[i] = in1[i-s_len];
	}
	return 0;
}
