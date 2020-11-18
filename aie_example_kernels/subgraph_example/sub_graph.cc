#include <adf.h>
#include "kernels.h"

using namespace adf;

/*
 * adf dataflow graph to compute weighted moving average of 
 * the last 8 samples in a stream of numbers
 */

class simpleGraph : public adf::graph
{
private:
  adf::kernel k1;
  adf::kernel k2;

public:
  adf::port<input> in;
  adf::port<output> out;
  
  simpleGraph()
  {
    // create kernels
    k1 = kernel::create(weighted_sum);
    k2 = kernel::create(average_div);
    
    // create nets to connect kernels and IO ports
    connect< window<128> > net0 (in, k1.in[0]);
    connect< window<128> > net1 (k1.out[0], k2.in[0]);
    connect< window<128> > net2 (k2.out[0], out);

    // specify kernel sources
    source(k1) = "weighted_sum.cc";
    source(k2) = "average_div.cc";

    // specify kernel run times
    runtime<ratio>(k1) = 0.1;
    runtime<ratio>(k2) = 0.1;
  }
};

