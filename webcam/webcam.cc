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
	printf("%s : open video %s\n",__FUNCTION__, dev_name);
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
	req.count = 2;
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
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);
	} while ((r == -1 && (errno = EINTR)));
	if (r == -1) {
		printf("select %s",strerror(errno));
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
			int p = ctl.read();
			if (p) paused = true;
			else paused = false;
			if (paused) continue; // don't send enything more for processing
		}
		// read from captured buffer convert it & send it out for processing
		cv::Mat *s_img = wc_db.start_reading();
		cv::Mat img;
		cvt(*s_img, img);
		wc_db.end_reading();
		if (s_img->empty()) {
			printf("Image empty will wait 1 second\n");
			sleep(1);
			continue;
		}
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
			int p = ctl.read();
			if (p) paused = true;
			else paused = false;
			if (paused) continue; // don't send enything more for processing
		}
		// read from captured buffer convert it & send it out for processing
		cv::Mat *s_img = wc_db.start_reading();
		cv::Mat img;
		cvt(*s_img, img);
		wc_db.end_reading();
		if (s_img->empty()) {
			printf("Image empty will wait 1 second\n");
			sleep(1);
			continue;
		}
		if (s_img->isContinuous()) {
			memcpy(oa,s_img->data,s_img->total()*s_img->elemSize());
			cont.read(); // wait till pipe is ready
		} else {
			printf("Cannot handle non-contiguos image\n");
		}
	} while (1);
}

// reads the webcam's double buffer and writes it out into an output
// vsi_device
// ///////////////////////////////////////////////////////////////////
void webcam::webcam_cvt_process_image(vsi::device<int> &mem,
				      hls::stream<int> &ctl,
				      std::function<void (cv::Mat &,cv::Mat &)>const &cvt)
{
	// if it is a paused state then wait for it to be unpaused
	while (paused && ctl.empty()) return;
	// pause the processing if requested
	if (!ctl.empty()) {
		int p = ctl.read();
		if (p) paused = true;
		else paused = false;
		if (paused) return; // don't send enything more for processing
	}
	// read from captured buffer convert it & send it out for processing
	cv::Mat *s_img = wc_db.start_reading();
	if (s_img->empty()) {
		printf("Image empty will wait 1 second\n");
		sleep(1);
		return;
	}
	cv::Mat img;
	cvt(*s_img, img);
	wc_db.end_reading();
	if (img.isContinuous()) {
		mem.pwrite(img.data,img.total()*img.elemSize(),0);
	} else {
		printf("Cannot handle non-contiguos image\n");
	}
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

// ///////////////////////////////////////////////////////////////////
// mouse event call back function
// ///////////////////////////////////////////////////////////////////
static void gs_mouse_callback(int event, int x, int y, int flags, void *param)
{
	static int state = 0;
	cv::Mat img ;
	hls::stream<int> *ctrl = (hls::stream<int> *)param;

	if (event != CV_EVENT_LBUTTONDOWN) return;

	// forward then reverse
	if (state == 0) {
		state = 1;
	} else {
		state = 0;
	}
	ctrl->write(state); // send the state to anyone waiting
}

// ///////////////////////////////////////////////////////////////////
// implementation of the opencv display method: reads from the display
// double buffer of opencv_display class and call imshow
// ///////////////////////////////////////////////////////////////////
void opencv_display::opencv_display_image(hls::stream<int> &control)
{
	cv::namedWindow("User View 0");
	cv::setMouseCallback("User View 0",gs_mouse_callback,&control);
	while (1) {
		// read from doule buffer and display
		cv::Mat *img_data = wc_db[0].start_reading();
		if (!img_data->empty()) cv::imshow("User View 0",*img_data);
		wc_db[0].end_reading();
		int key = cv::waitKey(1);
		if (key != -1) control.write(key);
		usleep(10);
	}
}

void bounces(hls::stream<int> &control, int key)
{
	//bounce the two signals
	for(int i = 0; i < 10; i ++){
		control.write(0x52);
		//printf("bounce write %d\n", 0x52);
		control.write(0x54);
		//printf("bounce write %d\n", 0x54);
	}
	//now do the real key
	control.write(key);
	//printf("bounce write %d\n", key);
}

// ///////////////////////////////////////////////////////////////////
// implementation of the opencv display method: reads from the display
// double buffer of opencv_display class and call imshow
// ///////////////////////////////////////////////////////////////////
void opencv_display::opencv_display_image_bounces(hls::stream<int> &control)
{
	cv::namedWindow("User View 0");
	cv::setMouseCallback("User View 0",gs_mouse_callback,&control);
	while (1) {
		// read from doule buffer and display
		cv::Mat *img_data = wc_db[0].start_reading();
		if (!img_data->empty()) cv::imshow("User View 0",*img_data);
		wc_db[0].end_reading();
		int key = cv::waitKey(1);
		if (key != -1) bounces(control, key);
		usleep(10);
	}
}


#ifndef __VSI_HLS_SYN__

// ///////////////////////////////////////////////////////////////////
// a "filiter" to remove bounce from the control data
// ///////////////////////////////////////////////////////////////////
void debouncer(hls::stream<int> &in, hls::stream<int> &out)
{
	int last = -1;
	int read_val = -1;
	if(!in.empty()){
		sleep(.1);
		while (!in.empty()) {
			read_val = in.read();
			last = read_val;
		}
		out.write(last);
	}

}
#endif



// ///////////////////////////////////////////////////////////////////
// instantiate the Open_cv display
// ///////////////////////////////////////////////////////////////////
static opencv_display od("User_image0");


// ///////////////////////////////////////////////////////////////////
// Function to display the image
// ///////////////////////////////////////////////////////////////////
void display_image(hls::stream<int> &control) {
	od.opencv_display_image(control);
}

// ///////////////////////////////////////////////////////////////////
// Function to display the bounce image
// ///////////////////////////////////////////////////////////////////
void display_image_bounces(hls::stream<int> &control) {
	od.opencv_display_image_bounces(control);
}


// ///////////////////////////////////////////////////////////////////
// converts image to Black & white and resizes to required size
// ///////////////////////////////////////////////////////////////////
static void make_bw(cv::Mat &in_mat, cv::Mat &out_mat) {
	cv::Mat tmp_mat;
	cv::cvtColor(in_mat, tmp_mat, cv::COLOR_RGB2GRAY);
	cv::resize(tmp_mat,out_mat,cv::Size(WC_NCOLS/2,WC_NROWS/2));
}

// ///////////////////////////////////////////////////////////////////
// The main webcam function : instantiates a webcam starts the image
// capture for it. Call the webcam's convert process image to convert
// to Black & White  writes the image into the shared buffer. Then sends
// a start command to the image processing block in (depending on mode)
// and waits for the processing to complete. When complete it copies the
// image into the display double buffer and continues
// ///////////////////////////////////////////////////////////////////
void webcam0(hls::stream<int> &control,
	     vsi::device<int> &mem_mm, hls::stream<st> &start_mm, hls::stream<st> &done_mm,
	     vsi::device<int> &mem_fc, hls::stream<st> &start_fc, hls::stream<st> &done_fc
	     ) {
  	//printf("Webcam0 started\n");
	static uint8_t ib [WC_HIMGSIZE_BW];
	static webcam cam0 ("/dev/video0");
	int p_param = 0;
	static int mode = 0;
	hls::stream<int> ctl;
	cam0.webcam_capture_image();

	if (!control.empty()) {
		int c = control.read();
		if (c < 0x10) mode = c;
		p_param = c;
	}
	st s;
	s.data = 0;
	s.last = 1;
	//printf("%s mode = %d\n",__FUNCTION__,mode);

	// if q then exit
	if (p_param == 0x71) {
		//printf("%s: q key presses\n",__FUNCTION__);
		exit(0);
	}

	if (mode) {
		static int threshold = 300;
		//printf("%s: keypress: %d threshold: %d \n",__FUNCTION__, p_param, threshold);
		// if up-arrow then increase speed
		if (p_param == 0x52 && threshold > 100) {
			//printf("%s: Up Arrow %d\n",__FUNCTION__,threshold);
			threshold -= 10;
		}
		// if down arrow then decrease speed
		if (p_param == 0x54 && threshold < 300) {
			//printf("%s: Down Arrow %d\n",__FUNCTION__,threshold);
			threshold += 10;
		}
		cam0.webcam_cvt_process_image(mem_fc,ctl,make_bw);
		s.data = threshold;
		// send start to processing algo
		start_fc.write(s);
		st d = done_fc.read(); // wait for processing complete
		mem_fc.pread(ib,WC_HIMGSIZE_BW,0); // get the image from shared memory
	} else {
		cam0.webcam_cvt_process_image(mem_mm,ctl,make_bw);

		// send start to processing algo
		start_mm.write(s);
		st d = done_mm.read(); // wait for processing complete
		mem_mm.pread(ib,WC_HIMGSIZE_BW,0); // get the image from shared memory
	}

	cv::Mat im(WC_NROWS/2,WC_NCOLS/2, CV_8UC1,(unsigned char*)ib); // create a cv matrix

	// copy to the display double buffer of the opencv display
	cv::Mat *d_img = od.wc_db[0].start_writing();
	*d_img = im.clone();
	od.wc_db[0].end_writing();
	usleep(100);
}

#endif
