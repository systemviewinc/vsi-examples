#include <adf.h>

void distribute(input_stream_int32 * instream, output_stream_int32 * outstream) {
  int32 i;
  for (i=0;i<1024;i++) {
    int32 j = readincr(instream);
    writeincr(outstream, j);
  };
}

void distribute2(input_stream_int32 * instream, output_stream_int32 * outstream, output_stream_int32 * outstream2) {
  int32 i;
  for (i=0;i<1024;i++) {
    int32 j = readincr(instream);
    writeincr(outstream, j);
    writeincr(outstream2, j);
  };
}
