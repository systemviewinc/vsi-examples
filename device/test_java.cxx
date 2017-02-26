#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <vsi_device.h>
#include <chrono>
#include <map>
#include <hls_stream.h>
#include "dev_read_write.h"

void test_java(hls::stream<servo_command> &inc, hls::stream<servo_command> &outc) {
	while(1) {
		servo_command sc ;
		printf("Going to sleep\n");
		sleep(1);
		printf("sending command \n");
		sc.mode = 1;
		sc.angle = -1;
		sc.incr = 9;
		sc.delay = 1000;
		inc.write(sc);
		sc = outc.read();
		printf("Got back sc mode %d, angle %d, incr %d, delay %d\n",
		       sc.mode, sc.angle, sc.incr, sc.delay);
	}
}
