#include "stream_mux.h"

//#define ARB_ON_LAST

ap_axis_dkt<DATA_WIDTH> wout(hls::stream<ap_axis_dk<DATA_WIDTH> > &inp, ap_uint<1> tid)
{
	ap_axis_dk<DATA_WIDTH> in = inp.read();
	ap_axis_dkt<DATA_WIDTH> out;
	out.data = in.data;
	out.last = in.last;
	out.keep = in.keep;
	out.id   = tid;
	return out;
}

void stream_mux (hls::stream<ap_axis_dkt<DATA_WIDTH> > &in1,
		 hls::stream<ap_axis_dkt<DATA_WIDTH> > &in2,
		 hls::stream<ap_axis_dkt<DATA_WIDTH> > &outp)
{
	while (!in1.empty() || !in2.empty()) {
#pragma HLS PIPELINE II=1
		ap_axis_dkt<DATA_WIDTH> out;
		ap_axis_dkt<DATA_WIDTH> in;
		ap_uint<1> tid;
		bool set = false;
		if (!in1.empty()) {
			in = in1.read();
			tid = 0;
			set = true;
		} else if (!in2.empty()) {
			in = in2.read();
			tid = 1;
			set = true;
		}
		out.data = in.data;
		out.keep = in.keep;
		out.last = in.last;
		out.id   = tid;
		if (set) outp.write(out);
	}
}

void create_stream(hls::stream<ap_uint<DATA_WIDTH> >   &ind,
		   hls::stream<ap_axis_dkt<DATA_WIDTH> >&outs)
{
	static int id = 0;
	id++;
	// packets of 16 ap_uint<DATA_WIDTH>
	for (int i = 0 ; i < 8 ; i++) {
		ap_axis_dkt<DATA_WIDTH> out;
		out.data = ind.read();
		out.keep = (i == 7) ? 3 : -1;
		out.last = (i == 7);
		out.id   = id;
		outs.write(out);
	}
}

void strip_stream(hls::stream<ap_axis_dkt<DATA_WIDTH> > &ins,
		  hls::stream<ap_uint<DATA_WIDTH> >   &outd)
{
	while (!ins.empty()) {
		outd.write(ins.read().data);
	}
}

void stream_split_64(hls::stream<ap_axis_d<64> > &in_stream,
		       hls::stream<ap_axis_d<64> > &out_stream_1,
		       hls::stream<ap_axis_d<64> > &out_stream_2)
{
	ap_axis_d<64> idx;
	idx = in_stream.read();
	out_stream_1.write(idx);
	out_stream_2.write(idx);
}


void stream_join_64(hls::stream<ap_axis_d<64> > &in_stream_1,
		       hls::stream<ap_axis_d<64> > &in_stream_2,
		       hls::stream<ap_axis_d<64> > &out_stream)
{
	ap_axis_d<64> idx;
	idx = in_stream_1.read();
	idx = in_stream_2.read();
	out_stream.write(idx);
}


void stream_split_2_16(hls::stream<ap_axis_dk<16> > &ins,
		       hls::stream<ap_axis_dk<16> > &o1,
		       hls::stream<ap_axis_dk<16> > &o2)
{
	ap_axis_dk<16> idx;
	do {
#pragma HLS PIPELINE II=1
		idx = ins.read();
		o1.write(idx);
		o2.write(idx);
	} while (1);
}

#ifndef __VSI_HLS_SYN__


/**
 * @brief send data out in bursts
 *
 * @param in_stream direction=inout access=stream
 * @param out_mem direction=output access=memory
 */
void stream_to_mem (hls::stream<ap_axis_d<32> > &in_stream, int *out_mem)
{
	int in_data[1024];
	int idx = 0;
	printf("%s started\n",__FUNCTION__);
	while (idx < 1024) {
		ap_axis_d<32> e = in_stream.read();
		in_data[idx++] = e.data;
		if (e.last) break;
	}
	printf("%s sending to memory\n",__FUNCTION__);
	memcpy(out_mem,in_data,idx*sizeof(int));
	printf("%s  ended\n",__FUNCTION__);

}


/**
 * @brief receive data from "world" copy to memory and start next
 *
 * @param in_arr 	- input data
 * @param out_mem 	- pointer to external memory
 * @param start 	- start to next
 * @param resp 		- receive response
 */
void vsi_memory_ctl(int in_arr[1024],
		    vsi::device<int> &out_mem,
		    hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &start,
		    hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &resp)
{
	static int count = 0 ;
	int i;
	printf("%s started %d\n",__FUNCTION__,count);
	// perform operation

   out_mem.pwrite(in_arr, sizeof(int)*1024, 0);

	ap_axis<DATA_WIDTH, 0, 0, 0> w ;
	w.data = 1;
	w.last = 1;
	// tell next process something in memory
	start.write(w);
	printf("%s sent start waiting for response\n",__FUNCTION__);

	// wait for response
	ap_axis<DATA_WIDTH, 0, 0, 0> r = resp.read();
	printf("%s done %d\n",__FUNCTION__,count++);
}



void write_double_buffer (hls::stream<int> &dsize, ap_uint<64> buffer_storage [16][16]) {
	static int second_time = 0;
	int size = 0 ;
	if (second_time) while(1) sleep(10);
	for (int i = 0 ; i < 16 ; i++) {
		for (int j = 0 ; j < 16 ; j++) buffer_storage[i][j] = size++;
	}
	dsize.write(size);
	second_time = 1;
}

void read_double_buffer(hls::stream<int> &dsize, ap_uint<64> buffer_storage [16][16]) {
	int r_size = dsize.read();
	int size = 0;
	bool mis_match = false;
	for (int i = 0 ; i < 16 ; i++) {
		for (int j = 0 ; j < 16 ; j++) {
			if (buffer_storage[i][j] != size) {
				printf("Error mismatch @ %d %d got %d, expected %d\n",i,j,(int)buffer_storage[i][j],size);
				mis_match = true;
			}
			size++;
		}
	}
	if (!mis_match) printf("test passed\n");
	exit(1);
}

#endif

/**
 * @brief wait for data process data in the memory & say done
 *
 * @param start
 * @param mem
 * @param sdone
 */
 void vsi_process_data(hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &start,
                       hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &sdone,
                       int *mem,
                       int out_arr[1024]
 ) {
 	printf("%s started\n",__FUNCTION__);
 	ap_axis<DATA_WIDTH, 0, 0, 0> s = start.read(); // wait for data
 	int local_arr [1024];
 	if (s.data != 0) {
 	        //memcpy(local_arr,mem,sizeof(local_arr));
 		// process the data
 		for (int i = 0 ; i < 1024; i++) {
 			out_arr[i] = (mem[i] += 100);
 		}
 		//memcpy(mem,local_arr,sizeof(local_arr));
 	}
 	// tell we are done
 	ap_axis<DATA_WIDTH, 0, 0, 0> w;
 	w.data = 1;
	w.keep = -1;
 	w.last = 1;
 	sdone.write(w);
 	printf("%s done\n",__FUNCTION__);
 }

void vsi_process_data_stream(hls::stream<int> &start,
                      hls::stream<int> &sdone,
                      vsi::device<int> mem
) {
	printf("%s started\n",__FUNCTION__);
	int s = start.read(); // wait for data
	int local_arr [1024];
	mem.pread(local_arr, 4096, 0);
	if (s != 0) {
		// process the data
		for (int i = 0 ; i < 1024; i++) {
			//local_arr[i] = local_arr[i] + 100;
			sdone.write(local_arr[i]);
		}
	}
	// tell we are done
	printf("%s done\n",__FUNCTION__);
}

/**
 * @brief just a pass through
 *
 * @param in_arr
 * @param out_arr
 */
void pass_thru(int in_arr[1024], int out_arr[1024])
{
	printf("%s started\n",__FUNCTION__);
	for (int i =0 ; i < 1024; i++)
#pragma HLS PIPELINE II=1
		out_arr[i] = in_arr[i];
	printf("%s done\n",__FUNCTION__);
}

/**
 * @brief converts an array into memory
 *
 * @param in_arr
 * @param out_mem
 */
void array_to_mem(int in_arr[1024], int *out_mem)
{
	for (int i =0 ; i < 1024; i++)
		out_mem[i] = in_arr[i];
}

/**
 * @brief Stream pass thru
 *
 * @param ins
 * @param outd
 */
void pass_thru_streaming(
    hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &ins,
    hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &outd)
{
    ap_axis<DATA_WIDTH, 0, 0, 0> in;
    ap_axis<DATA_WIDTH, 0, 0, 0> out;

    while (!ins.empty()) {
        in = ins.read();

        out.data = in.data;
        out.last = in.last;
        out.keep = in.keep;

        outd.write(out);
    }
}

void inout_test(hls::stream<uint8_t> &input, hls::stream<uint8_t> &output)
{
	for (int i = 0 ; i < 1024; i ++) {
		uint8_t td = input.read();
		output.write(td);
	}
}

#ifndef __VSI_HLS_SYN__
void bulk_write(hls::stream<uint8_t> &outs, hls::stream<int> &cont)
{
	static uint8_t buff[1024*1024];
	while (1) {
		outs.write(buff,sizeof(buff));
		cont.read();
	}
}

void bul_read(hls::stream<uint8_t> &ins, hls::stream<int> &cont)
{
	static uint8_t buff[1024*1024];
	while (1) {
		ins.read(buff,sizeof(buff));
		usleep(10000);
		cont.write(1);
	}
}

#endif
