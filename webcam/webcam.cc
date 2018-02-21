// webcam.cc --- 
// 
// Filename: webcam.cc
// Description: 
// Author: Sandeep
// Maintainer: System View Inc
// Created: Thu Oct 12 13:55:50 2017 (-0700)
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
//  This file contains functions for opening a media camera
//  and reading buffers into a double buffer which can be
//  processed by a separate thread.
//  requires linking with -lv4l2 -lv4lconvert 

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


#ifndef __VSI_HLS_SYN__

#include "webcam.h"
typedef struct {
	int tracking;
	int tlx, tly;
	int brx, bry;
} track_data;

// ///////////////////////////////////////////////////////////////////
// reads a buffers and write it into a hls::stream
// ///////////////////////////////////////////////////////////////////
template <typename T, int im_size, int incr>
static void wc_convert_buff_2_hls_stream(unsigned char *buff, hls::stream<T> &outs)
{
	static T l_buff[im_size/incr];
	for (int idx = 0, li = 0 ; idx < im_size ; idx += incr, li++) {
		
		uint16_t tmp = 0 ;
		for (int i = 0 ; i < incr; i++)
			tmp += buff[idx+i];
		l_buff[li] = (tmp/incr);
		
	}
	outs.write(l_buff,(im_size/incr)*(sizeof(T)));
}

int webcam::xioctl(int fh, int request, void *arg)
{
	int r;
	
	do {
		r = v4l2_ioctl(fh, request, arg);
		if (r == -1 && ((errno == EINTR) || (errno == EAGAIN))) sleep(1);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));
	
	if (r == -1) {
		fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
	}
	return r;
}
// ///////////////////////////////////////////////////////////////////
// Constructor : uses /dev/video0 : change to another video source as
// reaeuired
// ///////////////////////////////////////////////////////////////////

webcam::webcam()
{
	//do real stuff
	fd = -1;
	dev_name = "/dev/video0";
	
	printf("%s : open video\n",__FUNCTION__);
	fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		printf("Cannot open device");
		exit(EXIT_FAILURE);
	}
	
	CLEAR(fmt);
	xioctl(fd, VIDIOC_G_FMT, &fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 640;
	fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	//fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;	
	xioctl(fd, VIDIOC_S_FMT, &fmt);
	if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
		printf("Libv4l didn't accept RGB24 format. driver outputs %c%c%c%c\n",
		       (fmt.fmt.pix.pixelformat >> 3*8) & 0xff,
		       (fmt.fmt.pix.pixelformat >> 2*8) & 0xff,
		       (fmt.fmt.pix.pixelformat >> 1*8) & 0xff,
		       (fmt.fmt.pix.pixelformat >> 0*8) & 0xff
		       );
		exit(EXIT_FAILURE);
	}
	printf("Driver supplied format %c%c%c%c\n",
	       (fmt.fmt.pix.pixelformat >> 3*8) & 0xff,
	       (fmt.fmt.pix.pixelformat >> 2*8) & 0xff,
	       (fmt.fmt.pix.pixelformat >> 1*8) & 0xff,
	       (fmt.fmt.pix.pixelformat >> 0*8) & 0xff);
	if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
		printf("Warning: driver is sending image at %dx%d\n",
		       fmt.fmt.pix.width, fmt.fmt.pix.height);
	
	CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	xioctl(fd, VIDIOC_REQBUFS, &req);
	printf("Got %d buffers\n",req.count);
	buffers = (buffer*)calloc(req.count, sizeof(*buffers));
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		CLEAR(buf);
		
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		
		xioctl(fd, VIDIOC_QUERYBUF, &buf);
		
		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
						     PROT_READ | PROT_WRITE, MAP_SHARED,
						     fd, buf.m.offset);
		
		if (MAP_FAILED == buffers[n_buffers].start) {
			printf("mmap failed cannot proceed");
			exit(EXIT_FAILURE);
		}
	}
	
	for (int i = 0; i < n_buffers; ++i) {
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		xioctl(fd, VIDIOC_QBUF, &buf);
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	xioctl(fd, VIDIOC_STREAMON, &type);
}

// ///////////////////////////////////////////////////////////////////
//  Destructor : munmap the buffers and close the video channel
// ///////////////////////////////////////////////////////////////////

webcam::~webcam()
{
	printf("%s : close video\n",__FUNCTION__);
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_STREAMOFF, &type);
	for (int i = 0; i < n_buffers; ++i)
		v4l2_munmap(buffers[i].start, buffers[i].length);

        v4l2_close(fd);
}

// ///////////////////////////////////////////////////////////////////
// Capture image and put it into double buffer
// ///////////////////////////////////////////////////////////////////
void webcam::webcam_capture_image()
{
	int fc = 0;
	do {
		do {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
		
			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			
			r = select(fd + 1, &fds, NULL, NULL, &tv);
		} while ((r == -1 && (errno = EINTR)));
		if (r == -1) {
			printf("select");
			//exit(1) ;
			return;
		}
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		do {
			r = xioctl(fd, VIDIOC_DQBUF, &buf);
			if (r==-1) sleep(1);
		} while (r == -1);
		// copy to the doubble buffer
		wc_db_t *dbw = wc_db.start_writing();
		memcpy(dbw->buff,(unsigned char*)buffers[buf.index].start,fmt.fmt.pix.sizeimage);
		wc_db.end_writing();
		
		// release the video buffer
		xioctl(fd, VIDIOC_QBUF, &buf);
		usleep(2500);
		if (fc++ == 100) {
			printf("%s: Got 100 more images\n",__FUNCTION__);
			fc = 0;
		}
	} while (1);
}

static webcam mywebcam;
static unsigned int wc_minmax[6];
static track_data g_track[4];

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
void webcam_save_minmax (hls::stream<int> &ins)
{
	for (int i = 0 ; i < 6 ; i++) {
		wc_minmax[i] = ins.read();
	}	
}

// /////////////////////////////////////////////////////////////////////////
//  thisfunction is invoked when the xf::MeanShift computation is complete
//  the hls::stream ins is expected to contain 4 trackdata in this order
//  the hls::stream cont tells the producer that the computation is complete
// /////////////////////////////////////////////////////////////////////////
void webcam_save_track (hls::stream<uint16_t> &ins, hls::stream<int> &cont)
{
	static int fc = 0;
	for (int i = 0;  i < 4 ; i++) {
		int j = (i == 0 ? i : i);
		g_track[j].tracking = ins.read();
		g_track[j].tly      = ins.read();
		g_track[j].tlx      = ins.read();
		g_track[j].bry      = ins.read();
		g_track[j].brx      = ins.read();		
		// if (g_track[j].tracking)
		// 	printf("%s: tracking %d (%d,%d) (%d,%d) \n",__FUNCTION__,
		// 	       g_track[j].tracking,
		// 	       g_track[j].tlx     , 
		// 	       g_track[j].tly     ,
		// 	       g_track[j].brx     , 
		// 	       g_track[j].bry     );
	}
	//printf("%s: got tracking data\n",__FUNCTION__);
	if (fc++ == 100) {
		printf("%s: got 100 more tracks\n",__FUNCTION__);
		fc = 0;
	}
	cont.write(1); // let pipeline continue
}

// ///////////////////////////////////////////////////////////////////
// Reads from the webcams double buffer and sends it out for further
// processing over a hls::stream outs. convert to color space  Will
// wait for response on the hls::stream cont to proceed with the next
// frame
// ///////////////////////////////////////////////////////////////////
void webcam_process_image_cvt(hls::stream<uint32_t> &outs, hls::stream<int> &cont)
{
	sleep(5);
	do {
		// read from captured buffer and send it out for processing
		wc_db_t *bp = mywebcam.wc_db.start_reading();
		cv::Mat img (WC_NROWS,WC_NCOLS,CV_8UC3,bp->buff);
		mywebcam.wc_db.end_reading();
		cv::Mat d_img;
		cv::Mat s_img;
		cv::cvtColor(img, s_img, cv::COLOR_BGR2RGBA);
		if (s_img.isContinuous()) {
			outs.write(s_img.data,s_img.total()*s_img.elemSize());
			cont.read(); // wait till pipe is ready
		} else {
			printf("Cannot handle non-contiguos image\n");
		}
		//printf("%s : got continue\n",__FUNCTION__);
		usleep(5000);
	} while (1);
}


// ///////////////////////////////////////////////////////////////////
// start the webcam capture into buffer
// ///////////////////////////////////////////////////////////////////
void webcam_start()
{
	mywebcam.webcam_capture_image();
}

// ///////////////////////////////////////////////////////////////////
//  Mouse button call back for hdmi image
// ///////////////////////////////////////////////////////////////////
static void webcam_mouse_callback(int event, int x, int y, int flags, void *userdata)
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
		if ((brx - tlx) < 20) brx = tlx + 20;
		if ((bry - tly) < 20) bry = tly + 20;
		printf("%s: sending tracking (%d,%d) (%d,%d)\n",
		       __FUNCTION__,tlx,tly,brx,bry);
		// g_track[0].tracking = 1;
		// g_track[0].tlx = tlx;
		// g_track[0].tly = tly;
		// g_track[0].brx = brx;
		// g_track[0].bry = bry;
		track_out->write(tly);
		track_out->write(tlx);
		track_out->write(bry);
		track_out->write(brx);
	}
}

ProducerConsumerDoubleBuffer<cv::Mat> wc_ddb;

// ///////////////////////////////////////////////////////////////////
// display the webcam image on a window
// ///////////////////////////////////////////////////////////////////
void webcam_display_image(hls::stream<uint16_t> &track_out)
//void webcam_opencv_display()
{
	int fc = 0;
	sleep(5);
	printf("%s:started\n",__FUNCTION__);
	//cv::namedWindow("webcam", 1);
	//cv::namedWindow("debug",1);
	cv::setMouseCallback("webcam",webcam_mouse_callback,&track_out);
	while (1) {
		// read from double buffer
		wc_db_t *dbr = mywebcam.wc_db.start_reading();
		cv::Mat simage(WC_NROWS,WC_NCOLS,CV_8UC3,dbr->buff); // copy into cv::mat
		mywebcam.wc_db.end_reading();
		cv::Mat image;
		cv::cvtColor(simage,image,cv::COLOR_BGR2RGB);
		// draw rectangles around min and max brightness areas
		for (int i = 0 ; i < 4; i++) {
			if (g_track[i].tracking) {
				cv::Rect rect_min(g_track[i].tlx,g_track[i].tly,
						  g_track[i].brx-g_track[i].tlx,
						  g_track[i].bry-g_track[i].tly);
				cv::rectangle(image,rect_min,cv::Scalar(0,255,255),2,8);
			}
		}
		//cv::Rect rect_max((wc_minmax[4]*8)-25,(wc_minmax[5]*8)-25,50,50);
		cv::circle(image,cv::Point(wc_minmax[4]*8,wc_minmax[5]*8),25,cv::Scalar(0,255,0),2,8);

		// display it
		cv::Mat *d_img = wc_ddb.start_writing();
		*d_img = image.clone();
		wc_ddb.end_writing();
		usleep(20000);
		if (fc++ == 100) {
			printf ("%s: Display 100 more images\n",__FUNCTION__);
			fc = 0;
		}
	}
}

extern ProducerConsumerDoubleBuffer<cv::Mat> mic_ddb;
extern ProducerConsumerDoubleBuffer<cv::Mat> wc_ddb;
extern ProducerConsumerDoubleBuffer<cv::Mat> flir_ddb;
void opencv_display_thread()
{
	cv::namedWindow("webcam",1);
	cv::namedWindow("microphone",1);
	cv::namedWindow("flir",1);
	//cv::namedWindow("debug",1);
	sleep(3);
	while(1) {
		if (mic_ddb.m_wc) {
			cv::Mat *mic_dimg = mic_ddb.start_reading();
			if (!mic_dimg->empty()) cv::imshow("microphone",*mic_dimg);
			mic_ddb.end_reading();
		}
		if (wc_ddb.m_wc) {
			cv::Mat *wc_dimg = wc_ddb.start_reading();
			if (!wc_dimg->empty()) cv::imshow("webcam",*wc_dimg);
			wc_ddb.end_reading();
		}
		if (flir_ddb.m_wc) {
			cv::Mat *flir_dimg = flir_ddb.start_reading();
			if (!flir_dimg->empty()) cv::imshow("flir",*flir_dimg);
			flir_ddb.end_reading();
		}
		
		cv::waitKey(1);
		usleep(5000);
	}
}

#endif
// 
// webcam.cc ends here
