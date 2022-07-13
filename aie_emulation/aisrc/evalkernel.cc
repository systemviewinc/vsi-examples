#include <adf.h>

void pipekernel ( input_stream_int32 * input_data,
                  output_stream_int32 * output_data
) {
    int32 sample =  readincr(input_data);
    writeincr(output_data, sample, true);
}

