#include <hls_stream.h>
#include <unistd.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include "xadc_driver.h"
#include <vsi_device.h>
#ifndef __VSI_HLS_SYN__
#include <chrono>
#include <ctime>
#endif
//#define BULK_READ
void xadc_control (vsi::device &adc_ctrl)
{
	unsigned int reset=XSM_SRR_IPRST_MASK;
	sleep(3); // wait for things to settle
	adc_ctrl.pwrite(&reset,sizeof(reset),XSM_SRR_OFFSET); // reset the core
	printf("Reset the fifo\n");
	while (1) sleep(10); // sleeep this thread forever
}

void xadc_data (hls::stream<xadc_t<16> > &adc_data)
{
#ifdef  BULK_READ
	static uint8_t  buff[3000000];
#else		
	static uint16_t buff[1000000];
#endif	
	bool err_exit = false;
	std::chrono::microseconds w_time ;
	while (1) {
		auto t_start = std::chrono::high_resolution_clock::now();
#ifdef BULK_READ
		uint8_t *bp = buff;
		int len = 0;
		int remain = sizeof(buff);
		while (len < sizeof(buff)) {
			int rlen = adc_data.read(bp,remain);
			remain -= rlen;
			len += rlen;
			bp  += rlen;
		}
#else		
		for (int i = 0 ; i < 1000000; i++) {
			xadc_t<16> d = adc_data.read();
			buff[i] = d.data;
			if (d.id) {
				err_exit = true;
			}
		}
#endif		
		auto t_end   = std::chrono::high_resolution_clock::now();
		w_time  = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start);
		if (err_exit) exit(-1);
		std::cout << "Received 1000000 samples in " << w_time.count() << " microSeconds " << std::endl;
	}
}


void timer_rate_check(vsi::device &atm)
{
	// calibrate
	// start timer in counter mode and check
	unsigned int TCSR0, TCR0 , TLR0;
	unsigned int TCSR1, TCR1 , TLR1, rv;
	sleep(1);
	TLR0 =  0; // clear the reload register
	atm.pwrite(&TLR0,sizeof(TLR0),4); // offset 4

	TCSR0 =  1<<8 ; // clear interrupt
	TCSR0 |= 1<<5 ; // load counter from TLR0
	atm.pwrite(&TCSR0,sizeof(TCSR0),0); // offset 0

	TCSR0 =  1<<7;	// start the timer
	atm.pwrite(&TCSR0,sizeof(TCSR0),0); // offset 0

	std::chrono::high_resolution_clock::time_point t2;
	std::chrono::microseconds diff_time;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	do {
		t2 = std::chrono::high_resolution_clock::now();
		atm.pread (&TCR0,sizeof(TCR0), 8); // read counter : offset 8
		diff_time = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
	} while (diff_time.count() < 1000000) ; // for 1 seconds

	// TCR0 has timer value for .5 seconds
	printf("%s: timer cycles %d for %lld microseconds\n",__FUNCTION__,TCR0,diff_time.count());
	while(1) sleep(10);
}
