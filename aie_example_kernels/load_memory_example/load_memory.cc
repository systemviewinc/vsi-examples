#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <cardano.h>
//#include "kernels.h"

extern int32 lut0[6*1024];  // local memory for master
extern void masterKernel(input_stream_int32 * s1,
			 output_window_int32 *o0,
			 output_window_int32 *o1,
			 output_window_int32 *o2, 
			 output_stream_int32 * os1);
extern void slaveKernel(input_window_int32 * s1);

class myGraph : public cardano::graph {
 private:
	cardano::kernel master;
	cardano::kernel slaveS;
	cardano::kernel slaveW;
	cardano::kernel slaveN;
	cardano::parameter l0; 
public:
	cardano::port<input> in;     // public interface to the graph
	cardano::port<output> out;   // is streaming interface 
	myGraph() {
		master = cardano::kernel::create(masterKernel);
		cardano::source(master) = "master.cc";
		cardano::runtime<ratio>(master) = 0.9;
		single_buffer(master.out[0]);  // double buffer will run out of space
		single_buffer(master.out[1]);
		single_buffer(master.out[2]);
		l0 = cardano::parameter::array(lut0);
		cardano::connect<>(l0,master);
		slaveS = cardano::kernel::create(slaveKernel);
		single_buffer(slaveS.in[0]);
		cardano::source(slaveS) = "slave.cc";
		slaveW = cardano::kernel::create(slaveKernel);
		cardano::source(slaveW) = "slave.cc";
		single_buffer(slaveW.in[0]);
		slaveN = cardano::kernel::create(slaveKernel);
		cardano::source(slaveN) = "slave.cc";
		single_buffer(slaveN.in[0]);
		cardano::runtime<ratio>(slaveS) = 0.9;
		cardano::runtime<ratio>(slaveW) = 0.9;
		cardano::runtime<ratio>(slaveN) = 0.9;
		
		// use of async keyword shows that lock primitives are 
		// managed by the user.  Locks are still allocated by the compiler. 
		
		cardano::connect<cardano::stream> net0(in, master.in[0]);
		cardano::connect<cardano::stream> net1(master.out[3], out);
		cardano::connect<cardano::window<6*1024>> net2(async(master.out[0]), 
							       async(slaveS.in[0]));
		cardano::connect<cardano::window<6*1024>> net3(async(master.out[1]), async(slaveW.in[0]));
		cardano::connect<cardano::window<6*1024>> net4(async(master.out[2]), async(slaveN.in[0]));
	}
};
#endif
