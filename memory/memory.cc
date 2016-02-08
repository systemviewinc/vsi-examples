#include <stdio.h>
#include <string.h>
void vsi_memory(int in_arr[1024], int *out_mem, int out_arr[1024])
{
	int i_buff[1024];
	int i;
	// copy from memory
	memcpy(i_buff,out_mem,sizeof(i_buff)); 
	// perform operation
	for (i = 0 ; i < 1024; i++) {
		i_buff [i] += in_arr[i];
		out_arr[i] = i_buff[i];
	}
	// copy back to external memory
	memcpy(out_mem,i_buff,sizeof(i_buff));
}
