
#include "flir_lepton.h"
#ifndef __VSI_HLS_SYN__
#include <unistd.h>
#include "double_buffer.h"

#include <mutex>

typedef struct frb { uint16_t fb [NCOLS*NROWS + 1]; int32_t min_max[6]; } frame_buffer;
static ProducerConsumerDoubleBuffer<frame_buffer> flir_db;

void flir_dev_init(vsi::device &flir_control)
{
	static bool inited = false;
	if (!inited) {
		int wv = 0 ;
		flir_control.pwrite(&wv,sizeof(wv),0); // start the capture
		inited = true;
		printf("Initialized \n");
	}
	while (1) sleep(10);
}

void flir_lepton (hls::stream<ap_axis_dk<16> > &cam_s)
{
	static int fc = 0;
	int loc = 0 ;
	ap_axis_dk<16> vsd;

	frame_buffer *frame = flir_db.start_writing();	
	do {
		vsd = cam_s.read();
		if (vsd.last) {
			flir_db.end_writing();
			if (fc++ == 100) {
				//printf("Got 100 more images %d\n",loc);				
				fc = 0;
			}
			loc = 0;
			frame = flir_db.start_writing();			
		}
		frame->fb[loc++] = vsd.data;
		if (loc >= (NROWS*NCOLS + 1)) {
			printf("No Last loc reset \n");
			flir_db.end_writing();
			loc = 0;
			frame = flir_db.start_writing();
		}
	} while(1);
}

void flir_get_image(uint16_t *buffp)
{
	frame_buffer *rf = flir_db.start_reading();
	memmove(buffp, rf->fb, FRAME_SIZE);
	flir_db.end_reading();
}
#endif
