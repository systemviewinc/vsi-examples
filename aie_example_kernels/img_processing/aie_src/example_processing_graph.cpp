//
#include <cardano.h>
#include "kernels.h"

// using namespace cardano;

class kernel_graph : public cardano::graph {
private:
  cardano::kernel comp;

public:
  cardano::port<input> in;
  cardano::port<output> out_0;

  kernel_graph( ) { 
    comp =  cardano::kernel::create(compute);

    // Stream Connections
    cardano::connect< cardano::stream > extern_to_master(in, comp.in[0]);
    cardano::connect< cardano::stream > data_0_to_extern(comp.out[0], out_0);

    // Kernel sources
    cardano::source(comp) = "compute.cc";
    cardano::runtime<cardano::ratio>(comp) = 0.7;
  };
};

