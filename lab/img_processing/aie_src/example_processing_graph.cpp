//
#include <adf.h>
#include "kernels.h"

// using namespace adf;

class kernel_graph : public adf::graph {
private:
  adf::kernel comp;

public:
  adf::port<input> in;
  adf::port<output> out_0;

  kernel_graph( ) { 
    comp =  adf::kernel::create(compute);

    // Stream Connections
    adf::connect< adf::stream > extern_to_master(in, comp.in[0]);
    adf::connect< adf::stream > data_0_to_extern(comp.out[0], out_0);

    // Kernel sources
    adf::source(comp) = "compute.cc";
    adf::runtime<adf::ratio>(comp) = 0.7;
  };
};

