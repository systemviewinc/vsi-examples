// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

#include <adf.h>

void weighted_sum(input_window_int32 * in, output_window_int32 * out);
void weighted_sum_with_margin(input_window_int32 * in, output_window_int32 * out);
void average_div(input_window_int32 * in, output_window_int32 * out);

#endif


