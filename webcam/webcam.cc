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

webcam::webcam(const char *dev_name)
{
	//do real stuff
	fd = -1;
	running = false;
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
		// copy to the double buffer : convert to RGB
		cv::Mat *w_img = wc_db.start_writing();
		cv::Mat r_img (WC_NROWS,WC_NCOLS, CV_8UC3,(unsigned char*)buffers[buf.index].start);
		cv::cvtColor(r_img, *w_img, cv::COLOR_BGR2RGB);
		wc_db.end_writing();
		running = true;
		// release the video buffer
		xioctl(fd, VIDIOC_QBUF, &buf);
		usleep(2500);
	} while (1);
}

// ///////////////////////////////////////////////////////////////////
// Reads from the webcams double buffer and sends it out for further
// processing over a hls::stream outs. convert to color space  Will
// wait for response on the hls::stream cont to proceed with the next
// frame
// ///////////////////////////////////////////////////////////////////
void webcam::webcam_cvt_process_image(hls::stream<uint32_t> &outs,
				      hls::stream<int> &cont,
				      hls::stream<int> &ctl,
				      std::function<void (cv::Mat &,cv::Mat &)>const &cvt)
{
	do {
		// if it is a paused state then wait for it to be unpaused
		while (paused && ctl.empty()) usleep(1000);
		// pause the processing if requested
		if (!ctl.empty()) {
			int p = cont.read();
			if (p) paused = true;
			else paused = false;
			if (paused) continue; // don't send enything more for processing
		}
		// read from captured buffer convert it & send it out for processing
		cv::Mat *s_img = wc_db.start_reading();
		cv::Mat img;
		cvt(*s_img, img);
		wc_db.end_reading();

		if (s_img->isContinuous()) {
			outs.write(img.data,img.total()*img.elemSize());
			cont.read(); // wait till pipe is ready
		} else {
			printf("Cannot handle non-contiguos image\n");
		}
	} while (1);
}

// ///////////////////////////////////////////////////////////////////
// reads the webcam's double buffer and writes it out into an output
// array.
// ///////////////////////////////////////////////////////////////////
void webcam::webcam_cvt_process_image(uint32_t *oa,
				      hls::stream<int> &cont,
				      hls::stream<int> &ctl,
				      std::function<void (cv::Mat &,cv::Mat &)>const &cvt)
{
	do {
		// if it is a paused state then wait for it to be unpaused
		while (paused && ctl.empty()) usleep(1000);
		// pause the processing if requested
		if (!ctl.empty()) {
			int p = cont.read();
			if (p) paused = true;
			else paused = false;
			if (paused) continue; // don't send enything more for processing
		}
		// read from captured buffer convert it & send it out for processing
		cv::Mat *s_img = wc_db.start_reading();
		cv::Mat img;
		cvt(*s_img, img);
		wc_db.end_reading();
		if (s_img->isContinuous()) {
			memcpy(oa,s_img->data,s_img->total()*s_img->elemSize());
			cont.read(); // wait till pipe is ready
		} else {
			printf("Cannot handle non-contiguos image\n");
		}
	} while (1);
}

// ///////////////////////////////////////////////////////////////////
// display the webcam image on a window
// ///////////////////////////////////////////////////////////////////
void webcam::webcam_display_image()
{
	printf("%s:started\n",__FUNCTION__);
	while (1) {
		if (!paused) {
			// read from double buffer
			cv::Mat *t_image = wc_db.start_reading();
			cv::Mat image = t_image->clone();
			wc_db.end_reading();

			// if overlay image not empty then overlay
			o_lock.lock();
			if (!o_image.empty())
				image += o_image;
			o_lock.unlock();
			// copy to the display double buffer
			cv::Mat *d_img = wc_ddb.start_writing();
			*d_img = image.clone();
			wc_ddb.end_writing();
		}
		usleep(200);
	}
}

// webcam class definition ends

// opencvdisplay implementation
void opencv_display::opencv_start_display()
{
	while(1) {
		cv::Mat d_img;
		// read from doule buffer and display
	 	cv::Mat *img_data = wc_db.start_reading();
		if (!img_data->empty()) cv::imshow("User View",*img_data);
		wc_db.end_reading();
		cv::waitKey(1);
		usleep(500); // wait a little so other threads can proceed
	}
}


#endif
