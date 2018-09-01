#include <stdio.h>
#include <vsi_device.h>

char s[] = "Echo.";
int s_len = sizeof(s) -1;

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


void pass_thru_device(int in_arr[1024], vsi::device &dev_out)
{
	printf("%s started\n",__FUNCTION__);
#pragma HLS PIPELINE II=1
	dev_out.write(in_arr, 4096);
	printf("%s done\n",__FUNCTION__);
}

void echo_pass_thru_device(int in_arr[1024], vsi::device &dev_out)
{
	printf("%s started\n",__FUNCTION__);
#pragma HLS PIPELINE II=1
	dev_out.write(s, sizeof(s));
	dev_out.pwrite(in_arr, 4096, sizeof(s));
	printf("%s done\n",__FUNCTION__);
}
