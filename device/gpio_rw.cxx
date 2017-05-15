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
			cout << "ERROR expected " << wval << " got "
			     << rval << std::endl;
			exit(-1);
		}
		sleep(1); // wait a sec 
		wval++;
		if (wval == 0xffffffff) {
			cout << "SUCCESS : test finished " << std::endl;
		}
	}
}
