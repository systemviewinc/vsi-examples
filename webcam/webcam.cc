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


// ///////////////////////////////////////////////////////////////////
// reads a buffers and write it into a hls::stream
// ///////////////////////////////////////////////////////////////////
static void wc_convert_buff_2_hls_stream(unsigned char *buff, hls::stream<uint16_t> &outs, int size)
{
	static uint16_t l_buff[WC_IMGSIZE/3];
	for (int idx = 0, li = 0 ; idx < WC_IMGSIZE ; idx += 3, li++) {
		uint16_t tmp = buff[idx];
		tmp += buff[idx+1];
		tmp += buff[idx+2];
		l_buff[li] = (tmp/3);
		
	}
	outs.write(l_buff,(WC_IMGSIZE/3)*2);
}

void webcam::xioctl(int fh, int request, void *arg)
{
	int r;
	
	do {
		r = v4l2_ioctl(fh, request, arg);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));
	
	if (r == -1) {
		fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
		return;
	}
}
// ///////////////////////////////////////////////////////////////////
// Constructor : uses /dev/video0 : change to another video source as
// reaeuired
// ///////////////////////////////////////////////////////////////////

webcam::webcam()
{
	//do real stuff
	fd = -1;
	dev_name = "/dev/video4";
	
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
		printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
		exit(EXIT_FAILURE);
	}
	if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
		printf("Warning: driver is sending image at %dx%d\n",
		       fmt.fmt.pix.width, fmt.fmt.pix.height);
	
	v4lconvert_data = v4lconvert_create(fd);
	if (v4lconvert_data == NULL)
		printf("v4lconvert_create");
	if (v4lconvert_try_format(v4lconvert_data, &fmt, &src_fmt) != 0)
		printf("v4lconvert_try_format");
	xioctl(fd, VIDIOC_S_FMT, &src_fmt);
	dst_buf = (unsigned char*)malloc(fmt.fmt.pix.sizeimage);
	
	CLEAR(req);
	req.count = 2;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	xioctl(fd, VIDIOC_REQBUFS, &req);
	
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
		xioctl(fd, VIDIOC_DQBUF, &buf);
		try {
			if (v4lconvert_convert(v4lconvert_data,
					       &src_fmt,
					       &fmt,
					       (unsigned char*)buffers[buf.index].start, buf.bytesused,
					       dst_buf, fmt.fmt.pix.sizeimage) < 0) {
				if (errno != EAGAIN)
					printf("v4l_convert");
			}			
		} catch(...){}
		// copy to the doubble buffer
		wc_db_t *dbw = wc_db.start_writing();
		memcpy(dbw->buff,dst_buf,fmt.fmt.pix.sizeimage);
		wc_db.end_writing();
		
		// release the video buffer
		xioctl(fd, VIDIOC_QBUF, &buf);
		usleep(2500);
	} while (1);
}

static webcam mywebcam;
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
void webcam_mark_minmax (hls::stream<int> &ins, hls::stream<int> &cont)
{
	static unsigned int g_minmax_save[10][6];
	static int loc = 0;
	unsigned int minmax[6];
	for (int i = 0 ; i < 6 ; i++) minmax[i] = ins.read();
	
	memcpy(g_minmax,minmax,sizeof(minmax));
	// if (loc < 10) loc++;
	// else loc = 0;
	// // average out min max location from the last 10 samples
	// for (int i = 0 ; i < 6 ; i++) {
	// 	int avg = 0 ;
	// 	for (int j = 0; j < 10; j++) avg += g_minmax_save[j][i];
	// 	avg /= 10;
	// 	g_minmax[i] = avg;
	// }
	printf("%s: got minmax(%03d,%03d) (%03d,%03d) (%03d,%03d)\n",__FUNCTION__,
	       g_minmax[0],g_minmax[1],g_minmax[2],g_minmax[3],g_minmax[4],g_minmax[5]);
	cont.write(1); // let pipeline continue
}

// ///////////////////////////////////////////////////////////////////
// Reads from the webcams double buffer and sends it out for further
// processing over a hls::stream outs. Will wait for response on the
// hls::stream cont to proceed with the next frame.
// ///////////////////////////////////////////////////////////////////
void webcam_process_image(hls::stream<uint16_t> &outs, hls::stream<int> &cont)
{
	do {
		// read from captured buffer and send it out for processing
		wc_db_t *bp = mywebcam.wc_db.start_reading();
		wc_convert_buff_2_hls_stream(bp->buff,outs,WC_IMGSIZE);
		mywebcam.wc_db.end_reading();
		cont.read(); // wait till pipe is ready
		usleep(2500);
	} while (1);
}

// ///////////////////////////////////////////////////////////////////
// this is an externally callable API : will allocate a buffer copy
// image from the double buffer into it and send the pointer & length
// into the pointers provided
// ///////////////////////////////////////////////////////////////////
void webcam_getimage(unsigned char *buffp)
{
	static bool inited = false;
	
	char header[]="P6\n640 480 255\n";
	unsigned char* asil=buffp;
	
	// read from double buffer
	wc_db_t *dbr = mywebcam.wc_db.start_reading();
	// wc_db.end_reading();
	cv::Mat image(WC_NROWS,WC_NCOLS,CV_8UC3,dbr->buff); // copy into cv::mat
	mywebcam.wc_db.end_reading();

	// draw rectangles around min and max brightness areas
	cv::Rect rect_min(g_minmax[2],g_minmax[3],20,30);
	cv::rectangle(image,rect_min,cv::Scalar(0,0,255),1,8);
	cv::Rect rect_max(g_minmax[4],g_minmax[5],30,20);
	cv::rectangle(image,rect_max,cv::Scalar(255,0,255),1,8);

	uint8_t *lbuffer = asil+strlen(header);
	
	if (image.isContinuous()) {
		memcpy(asil+strlen(header), image.data, WC_IMGSIZE);
	} else {
		for (int r = 0 ; r < image.rows ; r++) {
			memcpy(lbuffer,image.ptr<uint8_t>(r),3*WC_NCOLS);
			lbuffer += 3*WC_NCOLS;
		}
	}
	memcpy(asil,header,strlen(header));
}

// ///////////////////////////////////////////////////////////////////
// start the webcam capture into buffer
// ///////////////////////////////////////////////////////////////////
void webcam_start()
{
	mywebcam.webcam_capture_image();
}

// ///////////////////////////////////////////////////////////////////
// display the webcam image on a window
// ///////////////////////////////////////////////////////////////////
void webcam_display_image()
{
	printf("Display image started\n");
	cv::namedWindow("webcam", 1);
	cv::namedWindow("hdmi", 1);
	while (1) {
		// read from double buffer
		wc_db_t *dbr = mywebcam.wc_db.start_reading();
		cv::Mat simage(WC_NROWS,WC_NCOLS,CV_8UC3,dbr->buff); // copy into cv::mat
		mywebcam.wc_db.end_reading();
		cv::Mat image;
		cv::cvtColor(simage,image,cv::COLOR_BGR2RGB);
		// draw rectangles around min and max brightness areas
		cv::Rect rect_min(g_minmax[2],g_minmax[3],20,30);
		cv::rectangle(image,rect_min,cv::Scalar(0,255,255),1,8);
		cv::Rect rect_max(g_minmax[4],g_minmax[5],30,20);
		cv::rectangle(image,rect_max,cv::Scalar(255,0,255),1,8);

		// display it
		cv::imshow("webcam", image);
		cv::waitKey(1);
	}
}


#endif
// 
// webcam.cc ends here
