#include <stdio.h>
/**
 * @brief just a pass through
 *
 * @param in_arr
 * @param out_arr
 */
void pass_thru_r5(int in_arr[1024], int out_arr[1024])
{
	printf("%s started\n",__FUNCTION__);
	for (int i =0 ; i < 1024; i++)
#pragma HLS PIPELINE II=1
		out_arr[i] = in_arr[i];
	printf("%s done\n",__FUNCTION__);
}
