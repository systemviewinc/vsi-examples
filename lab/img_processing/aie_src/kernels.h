// kernels.h - 
#ifndef KERNEL_H

#include <cardano/window/types.h>
#include <cardano/stream/types.h>

void compute(
  input_stream_int32 *data,
  output_stream_int32 *outstream
              );
#endif
