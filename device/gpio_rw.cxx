#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <vsi_device.h>
#include <chrono>
#include <map>
#include <ap_utils.h>
#include <hls_stream.h>
#include "ap_axi_sdata.h"


void gpio_loop_back(vsi::device &gpio_out, vsi::device &gpio_in)
{
	unsigned int rval, wval = 0xdeadbeef;
	while (1) {
		gpio_out.pwrite(&wval,sizeof(wval),0);
		gpio_out.pread(&rval,sizeof(rval),0);
		if (rval != wval) {
			std::cout << "ERROR expected " << wval << " got "
			     << rval << std::endl;
			exit(-1);
		}
		sleep(1); // wait a sec
		wval++;
		if (wval == 0xffffffff) {
			std::cout << "SUCCESS : test finished " << std::endl;
		}
	}
}

void led_8bit (vsi::device &led)
{
	unsigned int val = 1;
	while (1) {
		led.pwrite(&val,sizeof(val),0);
		sleep(1);
		if (val == 0x0080) val = 1;
		else val << 1;
	}
}

//This function will toggle a gpio high to reset a system at start
void device_reset (vsi::device &rst)
{
	unsigned int val = 1;
	rst.pwrite(&val,sizeof(val),0);
	sleep(1);
	unsigned int val = 0;
	rst.pwrite(&val,sizeof(val),0);
	//sleep forever
	while (1) sleep(100);
}

void device_control(vsi::device &dev)
{
	while (1) sleep(10);
}
