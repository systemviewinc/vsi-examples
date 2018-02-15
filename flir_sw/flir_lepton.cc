
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
	sleep(5); // delay startup
	if (!inited) {
		int wv = 0 ;
		flir_control.pwrite(&wv,sizeof(wv),0); // start the capture
		inited = true;
		printf("%s: Initialized \n",__FUNCTION__);
	}
	while (1) sleep(10);
}

void flir_lepton (hls::stream<ap_axis_dk<16> > &cam_s)
{
	static int fc = 0;
	int loc = 0 , min = 0xffff, max = 0;
	ap_axis_dk<16> vsd;

	frame_buffer *frame = flir_db.start_writing();	
	do {
		vsd = cam_s.read();
		if (vsd.last) {
			frame->min_max[0] = min;
			frame->min_max[1] = max;
			min = 0xffff; max = 0;
			flir_db.end_writing();
			loc = 0;
			frame = flir_db.start_writing();			
			if (fc++ == 100) {
				printf("%s: Got 100 more elements in q %d\n",__FUNCTION__,cam_s.size());				
				fc = 0;
			}
		}
		uint16_t ldata ;
		ldata = frame->fb[loc++] = vsd.data;
		if (ldata > max) max = ldata;
		if (ldata < min) min = ldata;
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

void flir_opencv_display()
{
	int fc = 0;
	sleep(5);
	while(1) {
		frame_buffer *rf = flir_db.start_reading();
		cv::Mat simg (NROWS,NCOLS,CV_16UC1,rf->fb);		
		flir_db.end_reading();
		cv::Mat nimg;
		cv::Mat dimg;
		cv::Mat cimg;
		
		cv::normalize(simg,nimg,0,255,cv::NORM_MINMAX,CV_8UC1); 
		cv::applyColorMap(nimg, cimg, cv::COLORMAP_HOT);		
		cv::resize(cimg,dimg,cv::Size(8*simg.cols,8*simg.rows));
		cv::imshow("flir",dimg);		
		cv::waitKey(10);
		usleep(10000);
		if (fc++ == 100) {
			printf ("%s: Display 100 more images\n",__FUNCTION__);
			fc= 0;
		}
	}
}
#endif
