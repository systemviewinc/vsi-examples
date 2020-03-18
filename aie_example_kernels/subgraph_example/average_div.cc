/* 
 * Kernel average_div
 *
 * Compute the weighted average by dividing each value by 36.
 */

#include <cardano.h>
#include <stdio.h>
#include "include.h"

void average_div(input_window_int32 * in, output_window_int32 * out) 
{
  for (unsigned i = 0; i < NUM_SAMPLES; i++) 
  {
    int32 c1;
    window_readincr(in, c1);
    window_writeincr(out, c1/36);
  }
}
