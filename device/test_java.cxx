#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <vsi_device.h>
#include <chrono>
#include <map>
#include <hls_stream.h>
#include "dev_read_write.h"

void test_java(hls::stream<servo_command> &inc) {
	static int num = 0;
	printf("Thread started\n");
	while(1) {
		servo_command sc ;
		sc = inc.read();
		num++;
		if (num == 10000) {
			printf("C++ Got bsc mode %d, angle %d, incr %d, delay %d\n",
			      sc.mode, sc.angle, sc.incr, sc.delay);
			num =0 ;
		}
	}
}
