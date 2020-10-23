#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <adf.h>
//#include "kernels.h"

extern int32 lut0[6*1024];  // local memory for master
extern void masterKernel(input_stream_int32 * s1,
			 output_window_int32 *o0,
			 output_window_int32 *o1,
			 output_window_int32 *o2, 
			 output_stream_int32 * os1);
extern void slaveKernel(input_window_int32 * s1);

class myGraph : public adf::graph {
 private:
	adf::kernel master;
	adf::kernel slaveS;
	adf::kernel slaveW;
	adf::kernel slaveN;
	adf::parameter l0; 
public:
	adf::port<input> in;     // public interface to the graph
	adf::port<output> out;   // is streaming interface 
	myGraph() {
		master = adf::kernel::create(masterKernel);
		adf::source(master) = "master.cc";
		adf::runtime<ratio>(master) = 0.9;
		single_buffer(master.out[0]);  // double buffer will run out of space
		single_buffer(master.out[1]);
		single_buffer(master.out[2]);
		l0 = adf::parameter::array(lut0);
		adf::connect<>(l0,master);
		slaveS = adf::kernel::create(slaveKernel);
		single_buffer(slaveS.in[0]);
		adf::source(slaveS) = "slave.cc";
		slaveW = adf::kernel::create(slaveKernel);
		adf::source(slaveW) = "slave.cc";
		single_buffer(slaveW.in[0]);
		slaveN = adf::kernel::create(slaveKernel);
		adf::source(slaveN) = "slave.cc";
		single_buffer(slaveN.in[0]);
		adf::runtime<ratio>(slaveS) = 0.9;
		adf::runtime<ratio>(slaveW) = 0.9;
		adf::runtime<ratio>(slaveN) = 0.9;
		
		// use of async keyword shows that lock primitives are 
		// managed by the user.  Locks are still allocated by the compiler. 
		
		adf::connect<adf::stream> net0(in, master.in[0]);
		adf::connect<adf::stream> net1(master.out[3], out);
		adf::connect<adf::window<6*1024>> net2(async(master.out[0]), 
							       async(slaveS.in[0]));
		adf::connect<adf::window<6*1024>> net3(async(master.out[1]), async(slaveW.in[0]));
		adf::connect<adf::window<6*1024>> net4(async(master.out[2]), async(slaveN.in[0]));
	}
};
#endif
