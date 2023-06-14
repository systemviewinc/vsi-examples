#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "complex.h"
#include <ap_int.h>
using namespace std;
/* 
The current input data that we have are between the range of:
min_value =  -1.998856
max_value =  290.000404

so we normalized all the value to the range  [0, 128) with the following formula:
normalized_value = (float(value) - min_value) / (max_value - min_value) * 128
then multiplying it by 2 ^ FRACTIONAL_BITS that for Q8.8, the FRACTIONAL_BITS is 8
*/
#define min_value -1.998856f
#define max_value  290.000404f
#define normalizeValue(x) \
    ((int)(((float(x) - min_value) / (max_value - min_value) * 128) * (1<<8)))
/* 
we should not represent the N_RANGE, N_PULSES and PFA_NOUT_RANGE as fixed point since they are the index of the arrays
*/
#define N_RANGE 128
#define N_PULSES 128
#define PFA_NOUT_RANGE 128
#define T_PFA (13)
#define NUMBER_RANGES_FOR_EACH_AIE (16)
#define FABS_OUTPUT_COORDS (normalizeValue(0.048923f))
// each AIE has the required data for 16 ranges (NUMBER_RANGES_FOR_EACH_AIE)
#define INPUT_DATA_I_SIZE NUMBER_RANGES_FOR_EACH_AIE*N_PULSES
/* 
1/pi = 0.318309886183791
*/
# define PI                normalizeValue(3.14159265358979323846f)
/*
Since this algorithm has some computation between index and fixed_pont values, 
like k*input_spacing in:
sinc_arg = (out_coord - (input_start+k*input_spacing)) * input_spacing_inv;

for this place, k should also be normalized between [0, 128) with the min and max data (of sar_interp1 input values)
and represented in Q8.8 format.
A table will be added to have the equivalent of each index in Q8.8 format, called k_in_q_1_15[N_RANGE]
*/
ap_int<8,8> k_in_q_1_15[N_RANGE] = {
    224, 336, 448, 560, 673, 785, 897, 1009, 1122, 1234, 1346, 1458, 1570, 1683, 1795, 1907, 2019, 2132, 2244, 2356, 2468, 2580, 2693, 2805, 2917, 3029, 3142, 3254, 3366, 3478, 3590, 3703, 3815, 3927, 4039, 4151, 4264, 4376, 4488, 4600, 4713, 4825, 4937, 5049, 5161, 5274, 5386, 5498, 5610, 5723, 5835, 5947, 6059, 6171, 6284, 6396, 6508, 6620, 6733, 6845, 6957, 7069, 7181, 7294, 7406, 7518, 7630, 7743, 7855, 7967, 8079, 8191, 8304, 8416, 8528, 8640, 8752, 8865, 8977, 9089, 9201, 9314, 9426, 9538, 9650, 9762, 9875, 9987, 10099, 10211, 10324, 10436, 10548, 10660, 10772, 10885, 10997, 11109, 11221, 11334, 11446, 11558, 11670, 11782, 11895, 12007, 12119, 12231, 12344, 12456, 12568, 12680, 12792, 12905, 13017, 13129, 13241, 13353, 13466, 13578, 13690, 13802, 13915, 14027, 14139, 14251, 14363, 14476
};
typedef struct {
        complex<ap_int<8,8>> data_i [INPUT_DATA_I_SIZE];
        ap_int<8,8> window [PFA_NOUT_RANGE];
        ap_int<8,8> input_coords_start[N_PULSES];
        ap_int<8,8> input_coords_spacing[N_PULSES];
        ap_int<8,8> output_coords[PFA_NOUT_RANGE];
} Interp1DataIn;
typedef struct {
        complex<ap_int<8,8>> resampled[NUMBER_RANGES_FOR_EACH_AIE*PFA_NOUT_RANGE];
} Interp1DataOut;
static inline ap_int<8,8> find_nearest_range_coord(
    ap_int<8,8> target_coord,
    ap_int<8,8> input_coord_start,
    ap_int<8,8> input_coord_spacing,
    ap_int<8,8> input_coord_spacing_inv);
static inline ap_int<8,8> sinc(ap_int<8,8> x)
{
    // since x is a Q8.8 fixed point, so should be compared with zero normalized between [0, 128) with the min and max data in Q8.8
        // if (x == 0) {
        if (x == normalizeValue(0)) {
        // return constant one normalized between [0, 128) with the min and max data in Q8.8
                return normalizeValue(1);
        } else {
                const ap_int<8,8> arg = PI * x;
                // return (ap_int<8,8>) sinf(arg) / arg;
                return sin(arg) / arg;
        }
}
//volatile complex<ap_int<8,8>> g_resampled;
//volatile ap_int<8,8> g_resampled_re, g_resampled_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/
void sar_interp1(
                        // complex<ap_int<8,8>>                 * __restrict__ resampled,
                const complex<ap_int<8,8>>         * __restrict__ data_i_0,
                    const ap_int<8,8>                            * __restrict__ window,
                    const ap_int<8,8>                            * __restrict__ input_coords_start,
                    const ap_int<8,8>                            * __restrict__ input_coords_spacing,
                    const ap_int<8,8>                            * __restrict__ output_coords,
                        // const complex<ap_int<8,8>> * __restrict__ data_i_1
            complex<ap_int<8,8>> * __restrict__ g_result
                        ) {
    int32_t p, r, k, rmin, rmax, window_offset, data_index;
    complex<ap_int<8,8>> zero_complex(normalizeValue(0), normalizeValue(0)), result(normalizeValue(0), normalizeValue(0));
    ap_int<8,8> sinc_arg, sinc_val, win_val;
    ap_int<8,8> input_spacing, input_start, input_spacing_inv;
    ap_int<8,8> scale_factor;
    /* 
    we shouldn't represent the PFA_N_TSINC_POINTS_PER_SIDE as fixed point since it is used for the index
    */
    const int32_t PFA_N_TSINC_POINTS_PER_SIDE = (T_PFA - 1)/2;
    const complex<ap_int<8,8>> * __restrict__ data ;
    /* for (p = 0; p < N_PULSES; ++p) */
    // 8 AIEs is enough, we use 64 AIEs
    // that is 8 times of each set ( 8sets)
    // that ends the outerloop be 128/8= 16 iteration
    for (p = 0; p < 16; ++p)
    {
        input_start = input_coords_start[p];
        input_spacing = input_coords_spacing[p];
        input_spacing_inv = normalizeValue(1) / input_spacing;
        scale_factor = FABS_OUTPUT_COORDS * input_spacing_inv;
        complex<ap_int<8,8>> accum;
        /* for (r = 0; r < PFA_NOUT_RANGE; ++r) */
        for (r = 0; r < 16; ++r)
        { 
            bool write_zero = false;  
            const ap_int<8,8> out_coord = output_coords[r];
            ap_int<8,8> nearest = find_nearest_range_coord(
                output_coords[r], input_start, input_spacing, input_spacing_inv);
            // instead of comparing with zero, comapring with actual -1 normalized and in fixed point
            // if (nearest < 0)
            if (nearest == normalizeValue(-1))
            {
                write_zero = true;
                g_result[p*16+r] = zero_complex;
                //  continue;
            } 
            /* find_nearest_range_coord should never return a value >= N_RANGE */
            /*
             * out_coord is bounded in [nearest, nearest+1], so we check
             * which of the two input coordinates is closest.
             */
            ap_int<8,8> write_incr = normalizeValue(0);
            if (abs(out_coord - (input_start + (nearest+ (normalizeValue(1)))*input_spacing)) <
                abs(out_coord - (input_start + (nearest)*input_spacing)))
            {
                write_incr = normalizeValue(1);
            }
            nearest = nearest + write_incr;
            // since nearest is a fixed_point value,
            // to use it as index, need to convert back to int
            int32_t nearest_as_index = (int32_t)nearest >> 8;
            rmin = nearest_as_index - PFA_N_TSINC_POINTS_PER_SIDE;
            if (rmin < 0) { rmin = 0; }
            rmax = nearest_as_index + PFA_N_TSINC_POINTS_PER_SIDE;
            if (rmax >= N_RANGE) { rmax = N_RANGE-1; }
            window_offset = 0;
            if (nearest_as_index - PFA_N_TSINC_POINTS_PER_SIDE < 0)
            {
                window_offset = PFA_N_TSINC_POINTS_PER_SIDE - nearest_as_index;
            }
            
            
            complex<ap_int<8,8>> accum_arr[NUMBER_RANGES_FOR_EACH_AIE];
            
            #pragma clang loop vectorize(enable)            
            for (k = 0; k < NUMBER_RANGES_FOR_EACH_AIE; ++k)
            {
                win_val = window[window_offset+(k-rmin)];
                sinc_arg = (out_coord - (input_start+ (k_in_q_1_15[k])*input_spacing)) * input_spacing_inv;
                sinc_val = sinc(sinc_arg);
                data = data_i_0;
                data_index = (p * NUMBER_RANGES_FOR_EACH_AIE) + k;
                if (write_zero) {
                    /* to not accessing wrong memory */
                    data_index = 0;
                }
                complex<ap_int<8,8>> new_accum;
                new_accum =  data[data_index] * (sinc_val * win_val);
                if (k<rmin || k> rmax) {
                    new_accum = zero_complex;
                                }
                accum_arr[k] = new_accum;
                
            }
            // for the logic of reduction, we shouldn't add the array's values to anything as initial ( it actually help the auto_vec to generate the correct constant)
            complex<ap_int<8,8>> accum(0,0);
            #pragma clang loop vectorize(enable)            
            for (k = 0; k < NUMBER_RANGES_FOR_EACH_AIE; ++k)
            {
                accum += accum_arr[k];
            }
            
           result = accum * scale_factor;
            g_result[p*16+r] = result;
        }
    }
}
/*
 * C89 does not include the family of round() functions, so
 * we include a simple implementation of round().  This version is
 * naive in the sense that it does not work for all inputs and
 * does not honor rounding modes specified by, e.g., fesetround().
 */
static inline ap_int<8,8> naive_round(ap_int<8,8> x)
{
        return  (x + normalizeValue(0.5));
}
ap_int<8,8> find_nearest_range_coord(
    ap_int<8,8> target_coord,
    ap_int<8,8> input_coord_start,
    ap_int<8,8> input_coord_spacing,
    ap_int<8,8> input_coord_spacing_inv)
{
    /*
     * Test for the target coordinate being out-of-bounds with respect to
     * the input coordinates.
     */
        if (target_coord < input_coord_start ||
            target_coord >= (input_coord_start + normalizeValue(N_RANGE-1)*input_coord_spacing)) {
                return normalizeValue(-1);
        }
        return naive_round((target_coord - input_coord_start) * input_coord_spacing_inv);
}
/**************************************************************************************************/
/*                                         top level function                                          */
/**************************************************************************************************/
void sar_interp1_top(ap_int<8,8> * __restrict__ in_data_0,
                            ap_int<8,8> * __restrict__ out_data) {
        Interp1DataIn * __restrict__ interp1datain = (Interp1DataIn * __restrict__)in_data_0;
    complex<ap_int<8,8>> * __restrict out_data_complex = (complex<ap_int<8,8>> * __restrict__)out_data;
    sar_interp1 (
                        interp1datain->data_i,
                        interp1datain->window,
                        interp1datain->input_coords_start,
                        interp1datain->input_coords_spacing,
                        interp1datain->output_coords,
            out_data_complex
             );
}
