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

so we normalized all the constant values to the range  [-1, 1) with the following formula:
normalized_value = (float(value) - min_value) / (max_value - min_value) * 2 - 1
then multiplying it by 2 ^ FRACTIONAL_BITS that for Q1.15, the FRACTIONAL_BITS is 15
*/

#define min_value -1.998856f
#define max_value  290.000404f
#define normalizeValue(x) \
    ((int)(((float(x) - min_value) / (max_value - min_value) * 2 - 1) * (1<<15)))


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

# define PI		normalizeValue(3.14159265358979323846f)

/*
Since this algorithm has some computation between index and fixed_pont values, 
like k*input_spacing in:
sinc_arg = (out_coord - (input_start+k*input_spacing)) * input_spacing_inv;

for this place, k should also be normalized between [-1, 1) with the min and max data (of sar_interp1 input values)
and represented in Q1.15 format.
A table will be added to have the equivalent of each index in Q1.15 format, called k_in_q_1_15[N_RANGE]
*/
ap_int<1,15> k_in_q_1_15[N_RANGE] = {
    -32319, -32095, -31871, -31646, -31422, -31197, -30973, -30748, -30524, -30299, -30075, -29851, -29626, -29402, -29177, -28953, -28728, -28504, -28279, -28055, -27831, -27606, -27382, -27157, -26933, -26708, -26484, -26260, -26035, -25811, -25586, -25362, -25137, -24913, -24688, -24464, -24240, -24015, -23791, -23566, -23342, -23117, -22893, -22669, -22444, -22220, -21995, -21771, -21546, -21322, -21097, -20873, -20649, -20424, -20200, -19975, -19751, -19526, -19302, -19077, -18853, -18629, -18404, -18180, -17955, -17731, -17506, -17282, -17058, -16833, -16609, -16384, -16160, -15935, -15711, -15486, -15262, -15038, -14813, -14589, -14364, -14140, -13915, -13691, -13467, -13242, -13018, -12793, -12569, -12344, -12120, -11895, -11671, -11447, -11222, -10998, -10773, -10549, -10324, -10100, -9875, -9651, -9427, -9202, -8978, -8753, -8529, -8304, -8080, -7856, -7631, -7407, -7182, -6958, -6733, -6509, -6284, -6060, -5836, -5611, -5387, -5162, -4938, -4713, -4489, -4265, -4040, -3816
};

typedef struct {
	complex<ap_int<1,15>> data_i [INPUT_DATA_I_SIZE];
	ap_int<1,15> window [PFA_NOUT_RANGE];
	ap_int<1,15> input_coords_start[N_PULSES];
	ap_int<1,15> input_coords_spacing[N_PULSES];
	ap_int<1,15> output_coords[PFA_NOUT_RANGE];
} Interp1DataIn;

typedef struct {
	complex<ap_int<1,15>> resampled[NUMBER_RANGES_FOR_EACH_AIE*PFA_NOUT_RANGE];
} Interp1DataOut;

static inline ap_int<1,15> find_nearest_range_coord(
    ap_int<1,15> target_coord,
    ap_int<1,15> input_coord_start,
    ap_int<1,15> input_coord_spacing,
    ap_int<1,15> input_coord_spacing_inv);

static inline ap_int<1,15> sinc(ap_int<1,15> x)
{
    // since x is a Q15 fixed point, so should be compared with zero normalized between [-1, 1) with the min and max data in Q15
	// if (x == 0) {
	if (x == normalizeValue(0)) {
        // return constant one normalized between [-1, 1) with the min and max data in Q15
		return normalizeValue(1);
	} else {
		const ap_int<1,15> arg = PI * x;
		// return (ap_int<1,15>) sinf(arg) / arg;
		return sin(arg) / arg;
	}
}

//volatile complex<ap_int<1,15>> g_resampled;
//volatile ap_int<1,15> g_resampled_re, g_resampled_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/

void sar_interp1(
			// complex<ap_int<1,15>> 		* __restrict__ resampled,
        	const complex<ap_int<1,15>> 	* __restrict__ data_i_0,
		    const ap_int<1,15>    			* __restrict__ window,
		    const ap_int<1,15>    			* __restrict__ input_coords_start,
		    const ap_int<1,15>    			* __restrict__ input_coords_spacing,
		    const ap_int<1,15>    			* __restrict__ output_coords,
			// const complex<ap_int<1,15>> * __restrict__ data_i_1
            complex<ap_int<1,15>> * __restrict__ g_result
			) {
    int32_t p, r, k, rmin, rmax, window_offset, data_index;
    complex<ap_int<1,15>> zero_complex(normalizeValue(0), normalizeValue(0)), result(normalizeValue(0), normalizeValue(0));
    ap_int<1,15> sinc_arg, sinc_val, win_val;
    ap_int<1,15> input_spacing, input_start, input_spacing_inv;
    ap_int<1,15> scale_factor;
    /* 
    we shouldn't represent the PFA_N_TSINC_POINTS_PER_SIDE as fixed point since it is used for the index
    */

    const int32_t PFA_N_TSINC_POINTS_PER_SIDE = (T_PFA - 1)/2;
    const complex<ap_int<1,15>> * __restrict__ data ;

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
        complex<ap_int<1,15>> accum;
        /* for (r = 0; r < PFA_NOUT_RANGE; ++r) */
        for (r = 0; r < 16; ++r)
        { 
            bool write_zero = false;  
            const ap_int<1,15> out_coord = output_coords[r];
            ap_int<1,15> nearest = find_nearest_range_coord(
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
            ap_int<1,15> write_incr = normalizeValue(0);
            if (abs(out_coord - (input_start + (nearest+(normalizeValue(1)))*input_spacing)) <
                abs(out_coord - (input_start + (nearest)*input_spacing)))
            {
                write_incr = normalizeValue(1);
            }
            nearest = nearest + write_incr;
            // since nearest is a fixed_point value,
            // to use it as index, need to convert back to int
            int32_t nearest_as_index = (int32_t)nearest >> 15;
            rmin = nearest_as_index - PFA_N_TSINC_POINTS_PER_SIDE;
            if (rmin < 0) { rmin = 0; }
            rmax = nearest_as_index + PFA_N_TSINC_POINTS_PER_SIDE;
            if (rmax >= N_RANGE) { rmax = N_RANGE-1; }

            window_offset = 0;
            if (nearest_as_index - PFA_N_TSINC_POINTS_PER_SIDE < 0)
            {
                window_offset = PFA_N_TSINC_POINTS_PER_SIDE - nearest_as_index;
            }
            
            
            complex<ap_int<1,15>> accum_arr[NUMBER_RANGES_FOR_EACH_AIE];
            
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
                complex<ap_int<1,15>> new_accum;
                new_accum =  data[data_index] * (sinc_val * win_val);
                if (k<rmin || k> rmax) {
                    new_accum = zero_complex;
				}

                accum_arr[k] = new_accum;
                
            }
            // for the logic of reduction, we shouldn't add the array's values to anything as initial ( it actually help the auto_vec to generate the correct constant)
            complex<ap_int<1,15>> accum(0,0);

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

static inline ap_int<1,15> naive_round(ap_int<1,15> x)
{
	return  (x + normalizeValue(0.5));
}

ap_int<1,15> find_nearest_range_coord(
    ap_int<1,15> target_coord,
    ap_int<1,15> input_coord_start,
    ap_int<1,15> input_coord_spacing,
    ap_int<1,15> input_coord_spacing_inv)
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
/* 					top level function					  */
/**************************************************************************************************/
void sar_interp1_top(ap_int<1,15> * __restrict__ in_data_0,
		            ap_int<1,15> * __restrict__ out_data) {
	Interp1DataIn * __restrict__ interp1datain = (Interp1DataIn * __restrict__)in_data_0;
    complex<ap_int<1,15>> * __restrict out_data_complex = (complex<ap_int<1,15>> * __restrict__)out_data;

    sar_interp1 (
			interp1datain->data_i,
			interp1datain->window,
			interp1datain->input_coords_start,
			interp1datain->input_coords_spacing,
			interp1datain->output_coords,
            out_data_complex
             );
}

