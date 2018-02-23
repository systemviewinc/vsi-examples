
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
				printf("%s: Got 100 more packets in q %d\n",__FUNCTION__,cam_s.size());				
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

void flir_send_image(hls::stream<uint16_t> &outs, hls::stream<int> &cont)
{
	while (1) {
		frame_buffer *rf = flir_db.start_reading();
		cv::Mat s_img (NROWS,NCOLS,CV_16UC1,rf->fb);		
		flir_db.end_reading();
		if (s_img.isContinuous()) {
			outs.write(s_img.data,s_img.total()*s_img.elemSize());
			cont.read(); // wait till pipe is ready
		} else {
			printf("%s:Cannot handle non-contiguos image\n",__FUNCTION__);
		}
		usleep(5000);
	}
}

static unsigned int g_minmax[6];

// ///////////////////////////////////////////////////////////////////
//  thisfunction is invoked when the xf:minMaxLoc computation is complete
//  the hls::stream ins is expected to contain 6 integers in this order
//   	[0] = min_value
//   	[1] = max_value
// 	[2] = minx loc
//	[3] = miny loc
//	[4] = maxx loc
//	[5] = maxy loc
//  the function applies average filter to reduce the high frequency noise
//  in the min max computation.
//  the hls::stream cont tells the producer that the compuation is complete
// ///////////////////////////////////////////////////////////////////
void flir_save_minmax (hls::stream<int> &ins, hls::stream<int> &outs, hls::stream<int> &cont)
{
	for (int i = 0 ; i < 6 ; i++) {
		g_minmax[i] = ins.read();
		outs.write(g_minmax[i]);
	}
	cont.write(1); // let pipeline continue
}

ProducerConsumerDoubleBuffer<cv::Mat> flir_ddb;
void flir_opencv_display()
{
	int fc = 0;
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
		
		// mark the max location
		uint16_t max_x = g_minmax[4]*8;
		uint16_t max_y = g_minmax[5]*8;
		//cv::Rect rect_max(max_x-25,max_y-25,50,50);
		cv::circle(dimg,cv::Point(max_x,max_y),25,cv::Scalar(0,255,0),2,8);
		
		cv::Mat *d_img = flir_ddb.start_writing();
		*d_img = dimg.clone();
		flir_ddb.end_writing();
		usleep(5000);
		if (fc++ == 100) {
			printf ("%s: Display 100 more images\n",__FUNCTION__);
			fc= 0;
		}
	}
}
#endif
