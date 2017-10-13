#ifndef __VSI_HLS_SYN__

#include "webcam.h"


static void xioctl(int fh, int request, void *arg)
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

webcam::webcam()
{
	//do real stuff
	fd = -1;
	dev_name = "/dev/video0";
	
	printf("%s : open video\n",__FUNCTION__);
	fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		printf("Cannot open device");
		//exit(EXIT_FAILURE);
		return;
	}
	
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 640;
	fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
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
	req.count = 3;
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

webcam::~webcam()
{
	printf("%s : close video\n",__FUNCTION__);
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_STREAMOFF, &type);
	for (int i = 0; i < n_buffers; ++i)
		v4l2_munmap(buffers[i].start, buffers[i].length);

        v4l2_close(fd);
}

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
		try{
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

static unsigned int g_minmax_save[10][6];
static int loc = 0;
static unsigned int g_minmax[6];
static webcam mywebcam;

static void convert_buff_2_hls_stream(unsigned char *buff, hls::stream<uint16_t> &outs, int size)
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

static void convert_hls_stream_2_buff(hls::stream<wc_vs> &ins, unsigned char *buff, int size)
{
	unsigned char *bp = buff;
	ins.read(buff,size);
}

void webcam_mark_minmax (hls::stream<int> &ins, hls::stream<int> &cont)
{
	unsigned int minmax[6];
	for (int i = 0 ; i < 6 ; i++) minmax[i] = ins.read();
	
	memcpy(g_minmax_save[loc],minmax,sizeof(minmax));
	if (loc < 10) loc++;
	else loc = 0;
	// average out min max location from the last 10 samples
	for (int i = 0 ; i < 6 ; i++) {
		int avg = 0 ;
		for (int j = 0; j < 10; j++) avg += g_minmax_save[j][i];
		avg /= 10;
		g_minmax[i] = avg;
	}
	// printf("%s: got minmax(%d,%d) (%d,%d) (%d,%d)\n",__FUNCTION__,
	//        g_minmax[0],g_minmax[1],g_minmax[2],g_minmax[3],g_minmax[4],g_minmax[5]);
	cont.write(1); // let pipeline continue
}

void webcam_process_image(hls::stream<uint16_t> &outs, hls::stream<int> &cont)
{
	do {
		// read from captured buffer and send it out for processing
		wc_db_t *bp = mywebcam.wc_db.start_reading();
		convert_buff_2_hls_stream(bp->buff,outs,WC_IMGSIZE);
		mywebcam.wc_db.end_reading();
		cont.read(); // wait till pipe is ready
		usleep(10000);
	} while (1);
}

void webcam_getimage(int &cmd, unsigned char **buffp, int *len)
{
	static bool inited = false;
	
	// initialize the first time.
	if (!inited && cmd == 0) {
		inited = true;
		return ;
	}
	if (inited && cmd == 2) {
		inited = false;
		return;
	}
	char header[]="P6\n640 480 255\n";
	unsigned char* asil=(unsigned char*)malloc(WC_IMGSIZE+strlen(header));
	
	// read from double buffer
	wc_db_t *dbr = mywebcam.wc_db.start_reading();
	// wc_db.end_reading();
	cv::Mat image(WC_NROWS,WC_NCOLS,CV_8UC3,dbr->buff); // copy into cv::mat
	mywebcam.wc_db.end_reading();

	// draw rectangles around min and max brightness areas
	cv::Rect rect_min(g_minmax[2],g_minmax[3],20,20);
	cv::rectangle(image,rect_min,cv::Scalar(0,255,255),1,8);
	cv::Rect rect_max(g_minmax[4],g_minmax[5],20,20);
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
	*buffp = asil;
	*len =  WC_IMGSIZE+strlen(header);
}

void webcam_start()
{
	mywebcam.webcam_capture_image();
}

#endif
// Synthesizable
#include "stream_mux.h"
#include "hls_stream.h"
#include "ap_int.h"
#include "common/xf_common.h"
#include "common/xf_utility.h"
#include "core/xf_min_max_loc.hpp"
#include "webcam.h"
void stream_split_vs (hls::stream<wc_vs> &ins, hls::stream<wc_vs> &o1, hls::stream<wc_vs> &o2)
{
	stream_split<wc_vs,WC_IMGSIZE>(ins,o1,o2);
}
//#define _USE_XF_OPENCV
void webcam_min_max(hls::stream<uint16_t> &ins,	hls::stream<int> &outs)
{
#pragma HLS inline self	

	uint16_t _min_locx,_min_locy,_max_locx,_max_locy;
	int32_t _min_val = 0xfffff,_max_val = 0, _min_idx = 0, _max_idx = 0;	
#ifdef _USE_XF_OPENCV	
 	int idx = 0;
	uint16_t img_data[WC_NROWS*WC_NCOLS+1];
 	for (idx = 0 ; idx < (WC_NROWS*WC_NCOLS) ; idx++ ) {
#pragma HLS PIPELINE II=1
		img_data[idx] = ins.read();
	}
 	xf::Mat<XF_16UC1,WC_NROWS,WC_NCOLS,XF_NPPC1> cam_mat(WC_NROWS,WC_NCOLS,img_data);
	xf::minMaxLoc<XF_16UC1,WC_NROWS,WC_NCOLS,XF_NPPC1>(cam_mat, &_min_val, &_max_val, &_min_locx, &_min_locy, &_max_locx, &_max_locy);
#else
	uint16_t img_data[WC_NROWS][WC_NCOLS];
 	for (int idy = 0 ; idy < WC_NROWS ; idy++ ) {
		for (int idx = 0 ; idx < WC_NCOLS ; idx++) {
			img_data[idy][idx] = ins.read();
		}
	}
	for (int idy = 1 ; idy < WC_NROWS-1; idy++) {
		for (int idx = 1; idx < (WC_NCOLS-1); idx++) {
#pragma HLS PIPELINE II=1
			uint32_t data = 0;
			data += img_data[idy][idx];
			data += img_data[idy][idx+1];
			data += img_data[idy][idx-1];
			data += img_data[idy+1][idx];
			data += img_data[idy+1][idx+1];
			data += img_data[idy+1][idx-1];
			data += img_data[idy-11][idx];
			data += img_data[idy-1][idx+1];
			data += img_data[idy-1][idx-1];
			data /= 9;
			if (data < _min_val) {
				_min_val = data;
				_min_locx = idx;
				_min_locy = idy;
			}
			if (data > _max_val) {
				_max_val = data;
				_max_locx = idx;
				_max_locy = idy;
			}
		}
	}
#endif
	// printf("%s: (%d,%d) (%d,%d), (%d,%d)\n",__FUNCTION__,
	//        _min_val,_max_val,_min_locx,_min_locy,_max_locx,_max_locy);
	outs.write((int)_min_val);
	outs.write((int)_max_val);
	outs.write((int)_min_locx);
	outs.write((int)_min_locy);
	outs.write((int)_max_locx);
	outs.write((int)_max_locy);
}

