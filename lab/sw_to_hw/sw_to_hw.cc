#include "sw_to_hw.h"
#define ARRAY_SIZE 32
#define DATA_WIDTH 32
#define BIG_ARRAY_SIZE 4096

//#define ARB_ON_LAST
void send_4k_data(hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &out_data) //
 {
	ap_axis<DATA_WIDTH, 0, 0, 0> out;
	int counter = 0;


	printf("Sending data %i bytes now!\n", (sizeof(ap_uint<32>) * BIG_ARRAY_SIZE));
	for(int i = 0; i < BIG_ARRAY_SIZE; i++){
		out.data = counter++;
		if(i == BIG_ARRAY_SIZE-1){
			out.last = 1;
		}
		else {
			out.last = 0;
		}
		out_data.write(out);

	}
	printf("Done!\n");
}

//#define ARB_ON_LAST
void send_data_stream(hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &out_data,
		 hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &in_data)
 {
	ap_axis<DATA_WIDTH, 0, 0, 0> out;
	ap_axis<DATA_WIDTH, 0, 0, 0> in;
	int recieved_bytes = 0;

	//data to send
	ap_uint<32> send_data[ARRAY_SIZE] = { 29393098, 40383264, 47528588, 77368085,
		82195010, 184068703, 200482986, 224167555, 245387688, 308219692, 317803646,
		328700548, 441251311, 481984813, 499245185, 516121994, 532515282, 540883093,
		572188829, 574153163, 599703069, 612318427, 668220455, 747053201, 762356408,
		838843982, 855222663, 855559780, 864835927, 878982744, 968806822, 970299975};

	ap_uint<32> recieve_data[ARRAY_SIZE];

	printf("Sending data %i bytes now!\n", (sizeof(ap_uint<32>) * ARRAY_SIZE));
	for(int i = 0; i < ARRAY_SIZE; i++){
		out.data = send_data[i];
		if(i == ARRAY_SIZE-1){
			out.last = 1;
		}
		else {
			out.last = 0;
		}
		out_data.write(out);

	}
	printf("Waiting for data!\n");

	for(int i = 0; i < ARRAY_SIZE; i++){
		in = in_data.read();
		recieve_data[i] = in.data;
		recieved_bytes += sizeof(ap_uint<32>);
		printf("Recieved %i bytes!\n", recieved_bytes);
		if(in.last == 1 && i != ARRAY_SIZE-1){
			printf("Last came before we expected!\n");
			break;
		}
		else if(in.last != 1 && i == ARRAY_SIZE-1){
			printf("Last is late!\n");
			break;
		}
	}
	printf("Recieved %i bytes!\n", recieved_bytes);
	exit(0);
}

/**
 * @brief wait for data process data in the memory & say done
 *
 * @param start
 * @param mem
 * @param sdone
 */
void process_data_stream(hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &input,
		      hls::stream<ap_axis<DATA_WIDTH, 0, 0, 0> > &output)
{
	ap_axis<DATA_WIDTH, 0, 0, 0> in;
	ap_axis<DATA_WIDTH, 0, 0, 0> out;
	int last_data = 0;

	int i = 0;
	//blocking wait for first data to come in
	in = input.read();

	while(!in.last)
	{
		out.data = in.data + last_data;
		out.last = in.last;
		output.write(out);
		last_data = in.data;
		in = input.read();

	}
	out.data = in.data + last_data;
	out.last = in.last;
	output.write(out);

	// do{
	// 	in = input.read();
	// 	out.data = in.data + last_data;
	// 	out.last = in.last;
	// 	output.write(out);
	// 	last_data = in.data;
	// }
	// while(!in.last);

}
