// kernels.h - 
#ifndef KERNEL_H

#include <adf/window/types.h>
#include <adf/stream/types.h>

void compute(
  input_stream_int32 *data,
  output_stream_int32 *outstream
              );
#endif
