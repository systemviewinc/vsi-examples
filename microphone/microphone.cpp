#include "microphone.h"
#ifndef __VSI_HLS_SYN__
#include "double_buffer.h"
#include <chrono>
#include <ctime>
#include <math.h>
std::chrono::duration<double,std::nano> r_time;
float samples_p_sec = 10.0;

void get_micsample(vsi::device &mic, hls::stream<fft_data_s> &os, hls::stream<int> &cont)
{
	uint32_t wv = 0;
	uint32_t cr = 0;
	mic.pwrite(&wv,sizeof(wv),0x70); // enable slave
	wv = 0x066; // reset fifos : master mode : enable core
	mic.pwrite(&wv,sizeof(wv),0x60);
	mic.pread(&cr,sizeof(cr),0x60);
	while (1) {
		r_time = std::chrono::duration<double,std::nano>::zero();
		auto t_start = std::chrono::high_resolution_clock::now();
		for (int i = 1 ; i <= SAMPLES; i++) {
			fft_data_s fft_d;
			wv = 0;	
			mic.pwrite(&wv,sizeof(wv),0x68); // dummy write
			do {
				mic.pread(&wv,sizeof(wv),0x64);
			} while (wv & 1);
			mic.pread(&wv,sizeof(wv),0x6c); // read value
			fft_d.data.re = (float)wv;
			fft_d.data.im = 0.0;
			fft_d.last = (i == SAMPLES);
			os.write(fft_d);
			//printf("%s: got value %d\n",__FUNCTION__,wv);
		}
		auto t_end   = std::chrono::high_resolution_clock::now();
		r_time = t_end - t_start;
		samples_p_sec = SAMPLES*1e9/r_time.count();
		cont.read() ; // wait for fft to complete
		//printf("%s Continuing\n",__FUNCTION__);
	}
}

typedef struct { float amplitude [SAMPLES/2], frequency[SAMPLES/2]; } ampl;

ProducerConsumerDoubleBuffer<ampl> ampl_db;
void recv_fft_data (hls::stream<fft_amp> &amp_in, hls::stream<int> &cont)
{
	int pc = 0, i = 0;
	bool print = true;
	ampl *ampl_buff = ampl_db.start_writing();
	while (1) {
		fft_amp amp_d = amp_in.read();
		ampl_buff->amplitude[i] = *((float *)&amp_d.data);
		ampl_buff->frequency[i] = (i*samples_p_sec)/SAMPLES;
		i++;
		if (amp_d.last) {
			ampl_db.end_writing();
			cont.write(1); // let the send continue
			i = 0 ;
			if (pc++ == 100) {
				pc = 0;
				printf("%s: Got 100 more packets\n",__FUNCTION__);
			} else print = false;
			ampl_buff = ampl_db.start_writing();
		}
	}
}

void get_mic_fft(float *amplitude, float *frequency)
{
	ampl *ampl_buff = ampl_db.start_reading();
	memcpy(amplitude,ampl_buff->amplitude,sizeof(ampl_buff->amplitude));
	memcpy(frequency,ampl_buff->frequency,sizeof(ampl_buff->frequency));
	ampl_db.end_reading();
}

void control_qspi(vsi::device &ctrl)
{
	do {
		std::cout << "\n" << "Press eny key to continue ..";
	} while (std::cin.get() != '\n');
	unsigned int c = 1;
	ctrl.pwrite(&c,sizeof(c),0x10);
}
#endif

void recv_fft_process_data (hls::stream<fft_data_s> &fft_ds, hls::stream<fft_amp> &amp_out)
{
	float amplitude [SAMPLES/2];
	int sc = 1 ;
	while (1) {
		fft_data_s data_s = fft_ds.read();
#pragma HLS PIPELINE II=1		
		if (sc <= SAMPLES/2) {
			fft_amp aout;
			
			float d = sqrt((data_s.data.re * data_s.data.re) + (data_s.data.im * data_s.data.im));
			aout.data = *((ap_uint<32> *)&d);
			aout.last = (sc == SAMPLES/2);
			amp_out.write(aout);
		}
		if (data_s.last) sc = 0;
		else sc++;
	}
}

void get_micsample_hw(vsi::device mic, unsigned int *control)// hls::stream<fft_data_s> &os, hls::stream<int> &cont)
{
	static int state  = 0;
	uint32_t wv = 0;
	uint32_t cr = 0;
	int internal_control = *control;
	switch (state) {
	case 0:
		if (internal_control == 1) // intialize;
			state = 1;
		break;
	case 1:
		mic.pwrite(&wv,sizeof(wv),0x70); // enable slave
		wv = 0x166; // inhibit master : reset fifos : master mode : enable core
		mic.pwrite(&wv,sizeof(wv),0x60);
		mic.pread(&wv,sizeof(wv),0x60);
		state = 2;
		break;

	case 2:
		if (internal_control == 2) // start reading
			state = 3;
		break;

	case 3:
		for (int i = 1 ; i <= SAMPLES; i++) {
			fft_data_s fft_d;
			wv = i;
			for (int j = 0 ; j < 4; j++)
				mic.pwrite(&wv,sizeof(wv),0x68); // dummy write
			mic.pread(&wv,sizeof(wv),0x60);
			wv &= ~(1 << 8); // enable master
			mic.pwrite(&wv,sizeof(wv),0x60);
			for (int j = 0 ; j < 4; j++) {
				do {
					mic.pread(&wv,sizeof(wv),0x64);
				} while (wv & 1); // wait receive ~empty
				mic.pread(&wv,sizeof(wv),0x6c); // read value
			}
			// disable master
			mic.pread(&wv,sizeof(wv),0x60);
			wv |= (1 << 8); // disable master
			mic.pwrite(&wv,sizeof(wv),0x60);
		}
		if (internal_control == 3) state = 4; // pause
		break;
	case 4:
		if (internal_control == 4) state = 3; // restart
		break;
	}
	*control = state;
}

void recv_mic_raw_data(hls::stream<fft_data_s> &os, hls::stream<int> &cont)
{
	for (int i = 1; i <= SAMPLES; i++) {
		fft_data_s fft_d = os.read();
		printf("Sample [%d] value %f\n",i,fft_d.data.re);
	}
	cont.write(1);
}

// #ifndef __VSI_HLS_SYN__
// #include "double_buffer.h"
// ProducerConsumerDoubleBuffer<cv::Mat> mic_ddb;
// void mic_opencv_display()
// {
// 	printf("%s: started\n",__FUNCTION__); 
// 	while (1) {
// 		ampl *ampl_buff = ampl_db.start_reading();
// 		std::vector<double> dataX, dataY;
// 		for (int i = 0 ; i < SAMPLES/2 ;i++) {
// 			dataY.push_back((double)ampl_buff->amplitude[i]);
// 			dataX.push_back((double)ampl_buff->frequency[i]);
// 		}
// 		ampl_db.end_reading();
// 		cv::Mat matX (dataX);
// 		cv::Mat matY (dataY);
// 		cv::Ptr<cv::plot::Plot2d> plot = cv::plot::createPlot2d(matX, matY);
// 		cv::Mat img;
// 		plot->render(img);
// 		cv::Mat *d_img = mic_ddb.start_writing();
// 		*d_img = img.clone();
// 		mic_ddb.end_writing();
// 		usleep(1000);
// 	}
// }
// #endif
