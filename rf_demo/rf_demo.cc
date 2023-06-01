#include "rf_demo.h"
#include <vsi_device.h>
#include <ap_int.h>
#include "hls_math.h"

#define DATA_WIDTH 16
#define DATA_WIDTH_DSP 128

#define DATA_WIDTH_SRC 256
#define DATA_WIDTH_SINK 128



void input_process(hls::stream<ap_uint<DATA_WIDTH_DSP> > &ins,
		  hls::stream<ap_uint<DATA_WIDTH_DSP> > &outd,
		  vsi::device<ap_uint<DATA_WIDTH_DSP> > mem)
{
	ap_uint<DATA_WIDTH_DSP> offset = 0;
	while(1) {
		ap_uint<DATA_WIDTH_DSP> write_buff[256];
		ap_uint<DATA_WIDTH_DSP> in_data;

		//write offset to track frames
		outd.write(offset);

		for (int i = 0 ; i < 256; i++) {
			in_data = ins.read();
			write_buff[i] = in_data;
			outd.write(in_data);
		}
		mem.pwrite(write_buff,sizeof(write_buff),offset);
		offset += sizeof(write_buff);
	}


}

void iq_grab(vsi::device<ap_uint<DATA_WIDTH_DSP> > mem,
		  hls::stream<ap_uint<DATA_WIDTH_DSP> > &outd,
		  hls::stream<ap_uint<DATA_WIDTH_DSP> > &meta_in)
{
	ap_uint<DATA_WIDTH_DSP> frame = meta_in.read();
	ap_uint<DATA_WIDTH_DSP> read_buff[256];

	mem.pread(read_buff,sizeof(read_buff),frame);

	for (int i = 0 ; i < 256; i++) {
		outd.write(read_buff[i]);
	}
}

void process_data(hls::stream<ap_uint<DATA_WIDTH_DSP> > &ins,
		  hls::stream<ap_uint<DATA_WIDTH_DSP> > &meta_out)
{
	//first beat of data is the frame
	int frame = ins.read();
	ap_uint<DATA_WIDTH_DSP> read_buff[256];
	ap_uint<DATA_WIDTH_DSP> in_data;
	for (int i = 0 ; i < 256; i++) {
		in_data = ins.read();
		read_buff[i] = in_data;
		if (i > 0 && read_buff[i] == read_buff[i-1]) {
			meta_out.write(frame);
		}
	}
}


void process_frame(hls::stream<ap_uint<DATA_WIDTH_DSP> > &frame_in)
{
	//read the frame address
	ap_uint<DATA_WIDTH_DSP> read_buff[256];
	ap_uint<DATA_WIDTH_DSP> in_data;
	std::cout << "Frame: " << std::endl;
	for (int i = 0 ; i < 256; i++) {
		in_data = frame_in.read();
		read_buff[i] = in_data;
		std::cout << read_buff[i] << " ";
	}
	std::cout << std::endl;

}
