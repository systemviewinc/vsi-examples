// xilinx_video_pipe.cc --- 
// 
// Filename: 	xilinx_video_pipe.cc
// Description: Hooks into the Xilinx Video Pipeline 
// Author: 	sandeep@systemviewinc.com
// Maintainer: 
// Created: 	Wed Jan 10 09:43:09 2018 (-0800)
// Version: 
// Package-Requires: ()
// Last-Updated: 
//           By: 
//     Update #: 0
// URL: 
// Doc URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change Log:
// 
// 
// 
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.
// 
// 

// Code:
// Non-Sythesizable
#ifndef __VSI_HLS_SYN__
#include <unistd.h>
#include <linux/videodev2.h>
extern "C" {
#include "platform.h"
#include "video.h"
};

#include "double_buffer.h"
#include <hls_stream.h>

static int  vsi_filter_init(struct filter_s *fs, const struct filter_init_data *fid);
static void vsi_filter_func(struct filter_s *fs,
			    unsigned short *frm_data_in, unsigned short *frm_data_out,
			    int height_in, int width_in, int stride_in,
			    int height_out, int width_out, int stride_out);

static const char *vsi_modes[] = {
	"SW"
};

static struct filter_ops ops = {
	.init = vsi_filter_init,
	.func = vsi_filter_func
};

static struct filter_s vsi_FS = {
	.display_text 	= "VSI Generic Filter",
	.dt_comp_string = "",
	.pr_file_name   = "",
	.pr_buf		= NULL,
	.fd		= -1,
	.mode		= 0,
	.ops		= &ops,
	.data		= NULL,
	.num_modes	= 1,
	.modes		= vsi_modes
};

//
// initializie the xilinx video pipeline
//
void xilinx_video_pipe(void)
{
	struct filter_tbl ft = {};
	struct vlib_config_data cfg = {};
	struct vlib_config config = {
		.vsrc		= 0, // first one (HDMI)
		.type		= 1, // m2m pipeline
	};
	cfg.dri_card_id	= 1;
	cfg.ft		= &ft;
	cfg.fmt_in 	= V4L2_PIX_FMT_YUYV;
	cfg.fmt_out 	= V4L2_PIX_FMT_YUYV;

	filter_type_register(&ft,&vsi_FS); // register the filter
	/* Initialize video library */
	int ret = vlib_init(&cfg);
	if (ret) {
		fprintf(stderr, "ERROR: vlib_init failed: %s\n", vlib_errstr);
		exit(-1);
	}
	// start the pipe
	ret = vlib_change_mode(&config);
	if (ret) {
		fprintf(stderr,"ERROR: %s\n", vlib_errstr);
		exit(-1);
	}
	while(1) sleep(10); // sleep forever
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "double_buffer.h"

#define XF_MAX_OBJECTS 4
#define HC_NROWS 480
#define HC_NCOLS 640
#define HC_IMGSIZE (3*HC_NROWS*HC_NCOLS)
typedef struct { unsigned char buff[HC_IMGSIZE+1];} hc_db_t;
ProducerConsumerDoubleBuffer<hc_db_t> hc_db;

static uint16_t track[XF_MAX_OBJECTS],
	tly[XF_MAX_OBJECTS], tlx[XF_MAX_OBJECTS],
	bry[XF_MAX_OBJECTS], brx[XF_MAX_OBJECTS];

static void vsi_filter_func(struct filter_s *fs,
			    unsigned short *frm_data_in, unsigned short *frm_data_out,
			    int height_in, int width_in, int stride_in,
			    int height_out, int width_out, int stride_out)
{
	cv::Mat dis;
	cv::Mat s_img;

	// convert image to 640x480 RGB24
	cv::Mat img(height_in,width_in,CV_8UC2, frm_data_in);
	cv::cvtColor(img,dis,cv::COLOR_YUV2BGR_YUY2);
	cv::resize(dis,s_img,cv::Size(640,480));
	
	// copy the image into the double buffer
	hc_db_t *hc_dbw = hc_db.start_writing();
	unsigned char *lbuff = hc_dbw->buff;

	if (s_img.isContinuous()) {
		memcpy(lbuff, s_img.data, HC_IMGSIZE);
	} else {
		for (int r = 0 ; r < s_img.rows ; r++) {
			memcpy(lbuff,s_img.ptr<uint8_t>(r),3*HC_NCOLS);
			lbuff += 3*HC_NCOLS;
		}
	}
	hc_db.end_writing();
	
	return;
}

static int vsi_filter_init(struct filter_s *fs,
			   const struct filter_init_data *fid)	
{
	return 0;
}

// //////////////////////////////////////////////////////////////// ///////////////////////////////////////////////////////////////////
// reads a buffers and write it into a hls::stream
// ///////////////////////////////////////////////////////////////////
static void hc_convert_buff_2_hls_stream(unsigned char *buff, hls::stream<uint16_t> &outs, int size)
{
	static uint16_t l_buff[HC_IMGSIZE/3];
	for (int idx = 0, li = 0 ; idx < HC_IMGSIZE ; idx += 3, li++) {
		uint16_t tmp = buff[idx];
		tmp += buff[idx+1];
		tmp += buff[idx+2];
		l_buff[li] = (tmp/3);
		
	}
	outs.write(l_buff,(HC_IMGSIZE/3)*2);
}

/////
// Send image out for processing : send in 8UC4 format : 
// ///////////////////////////////////////////////////////////////////
void hdmi_send_image(hls::stream<uint16_t> &outs, hls::stream<int> &cont)
{

	while (1) {
		cv::Mat s_img;
		hc_db_t *dbr = hc_db.start_reading();
		hc_convert_buff_2_hls_stream(dbr->buff,outs,HC_IMGSIZE);
		hc_db.end_reading();
		cont.read() ; // wait for the image to be processed
	}
}

// ///////////////////////////////////////////////////////////////////
// save the tracking info
// ///////////////////////////////////////////////////////////////////
static unsigned int g_minmax[6];
void hdmi_save_tracking(hls::stream<int> &tdata, hls::stream<int> &cont)
{
	for (int i = 0 ; i < 6 ; i++) g_minmax[i] = tdata.read();
	printf("%s: got minmax(%03d,%03d) (%03d,%03d) (%03d,%03d)\n",__FUNCTION__,
	       g_minmax[0],g_minmax[1],g_minmax[2],g_minmax[3],g_minmax[4],g_minmax[5]);

	cont.write(1); // send the continue to image pipeline
}

// ///////////////////////////////////////////////////////////////////
//  Mouse button call back for hdmi image
// ///////////////////////////////////////////////////////////////////
static void hdmi_mouse_callback(int event, int x, int y, int flags, void *userdata)
{
	hls::stream<uint16_t> *track_out = (hls::stream<uint16_t> *) userdata;
	static uint16_t tlx, tly, brx, bry;
	if (event == cv::EVENT_LBUTTONDOWN) {
		tlx = x;
		tly = y;
		brx = -1;
		bry = -1;
	} else if (event == cv::EVENT_LBUTTONUP) {
		if (x < tlx) {
			brx = tlx;
			tlx = x;
		} else brx = x;
		if (y < tly) {
			bry = tly;
			tly = y;
		} else bry = y;
		printf("%s: sending tlx %d, tly %d , brx %d, bry %d\n",
		       __FUNCTION__, tlx, tly, brx, bry);
		track_out->write(tlx);
		track_out->write(tly);
		track_out->write(brx);
		track_out->write(bry);		
	}
}

// ///////////////////////////////////////////////////////////////////
// display the hdmi image on a window
// ///////////////////////////////////////////////////////////////////
void hdmi_display_image()
{
	sleep(1);
	printf("Display HDMI image started\n");
	printf("Named window created\n");
	//	cv::setMouseCallback("hdmi",hdmi_mouse_callback,&track_out);
	while (1) {
		// read from double buffer
		if (hc_db.write_count == 0) {
			usleep(1000);
			continue;
		}
		hc_db_t *dbr = hc_db.start_reading();
		cv::Mat image(HC_NROWS,HC_NCOLS,CV_8UC3,dbr->buff); // copy into cv::mat
		hc_db.end_reading();
		
		// draw rectangles around tracked objects
		cv::Rect rect_min(g_minmax[2],g_minmax[3],20,20);
		cv::rectangle(image,rect_min,cv::Scalar(0,255,255),1,8);
		cv::Rect rect_max(g_minmax[4],g_minmax[5],20,20);
		cv::rectangle(image,rect_max,cv::Scalar(255,0,255),1,8);

		// display it
		if (image.data) {
			cv::imshow("hdmi", image);
			cv::waitKey(1);
		}
	}
}


//int main() 
// 	xilinx_video_pipe();
// 	cv::namedWindow( "w", 1);

// 	return 0;
// }
#endif // __VSI_HLS_SYN__
// 
// xilinx_video_pipe.cc ends here
