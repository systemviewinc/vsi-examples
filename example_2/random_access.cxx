#include <stdio.h>
void random_access (int in1[64/sizeof(int)], int out1[64/sizeof(int)]) 
{
	for (int i = 0 ; i < 64/sizeof(int); i++) {
		out1[i] = ~in1[(64/sizeof(int))-i-1];
	}
}
