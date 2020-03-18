/* 
 * Kernel weighted_sum
 *
 * Compute the weighted sum of the last 8 values.
 */

#include <cardano.h>
#include <stdio.h>
#include "include.h"

void weighted_sum(input_window_int32 * in, output_window_int32 * out) 
{  
  for (unsigned i = 0; i < NUM_SAMPLES; i++) 
  {
    int32 val;
    int32 wsum = 0;

    window_decr(in, 7);

    for (unsigned j = 1; j <= 8; j++)
    {
      window_readincr(in, val);

      if (i + j >= 8)
      {
        wsum = wsum + (j * val);
      }
    }

    window_writeincr(out, wsum);
  }
}

void weighted_sum_with_margin(input_window_int32 * in, output_window_int32 * out) 
{
  window_incr(in, 8);

  for (unsigned i = 0; i < NUM_SAMPLES; i++) 
  {
    int32 val;
    int32 wsum = 0;

    window_decr(in, 7);

    for (unsigned j = 1; j <= 8; j++)
    {
      window_readincr(in, val);
      wsum = wsum + (j * val);
    }

    window_writeincr(out, wsum);
  }
}
