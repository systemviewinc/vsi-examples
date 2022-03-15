#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
// #include "_complex.h"

#define N_RANGE 128
#define N_PULSES 128
#define PFA_NOUT_RANGE 128
#define T_PFA (13)
#define NUMBER_RANGES_FOR_EACH_AIE (8)
#define FABS_OUTPUT_COORDS (0.048923f)
// each AIE has the required data for 4 ranges (NUMBER_RANGES_FOR_EACH_AIE/2)
#define INPUT_DATA_I_SIZE NUMBER_RANGES_FOR_EACH_AIE*N_PULSES/2
# define PI		3.14159265358979323846f

typedef struct  { float re, im; } _complex;
typedef struct {
	_complex data_i [INPUT_DATA_I_SIZE];
	float window [PFA_NOUT_RANGE];
	float input_coords_start[N_PULSES];
	float input_coords_spacing[N_PULSES];
	float output_coords[PFA_NOUT_RANGE];
} Interp1DataIn;

typedef struct {
	_complex resampled[NUMBER_RANGES_FOR_EACH_AIE*PFA_NOUT_RANGE];
} Interp1DataOut;

static inline int find_nearest_range_coord(
    float target_coord,
    float input_coord_start,
    float input_coord_spacing,
    float input_coord_spacing_inv);

static inline float sinc(float x)
{
	if (x == 0) {
		return 1.0f;
	} else {
		const float arg = PI * x;
		// return (float) sinf(arg) / arg;
		return sinf(arg) / arg;
	}
}

// volatile _complex g_resampled;
// volatile float g_resampled_re, g_resampled_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/

void sar_interp1(
			// _complex 		* __restrict__ resampled,
        	 _complex 	* __restrict__ data_i_0,
		    const float    			* __restrict__ window,
		    const float    			* __restrict__ input_coords_start,
		    const float    			* __restrict__ input_coords_spacing,
		    const float    			* __restrict__ output_coords,
			  _complex * __restrict__ g_result
			) {
    int p, r, k, rmin, rmax, window_offset, data_index;
    //_complex result;
    float sinc_arg, sinc_val, win_val[NUMBER_RANGES_FOR_EACH_AIE];
    float input_spacing, input_start, input_spacing_inv;
    float scale_factor;

    const int PFA_N_TSINC_POINTS_PER_SIDE = (T_PFA - 1)/2;
    _complex * __restrict__ data ;

    /* for (p = 0; p < N_PULSES; ++p) */
    for (p = 0; p < 16; ++p)
    {
        input_start = input_coords_start[p];
        input_spacing = input_coords_spacing[p];
        input_spacing_inv = 1.0f / input_spacing;

        scale_factor = FABS_OUTPUT_COORDS * input_spacing_inv;
        _complex accum;

        /* for (r = 0; r < PFA_NOUT_RANGE; ++r) */
        for (r = 0; r < 16; ++r)
        {
            bool write_zero = true;
            const float out_coord = output_coords[r];
            int nearest = find_nearest_range_coord(
                output_coords[r], input_start, input_spacing, input_spacing_inv);
            if (nearest > 0)
            {
                write_zero = false;
            }
            /* find_nearest_range_coord should never return a value >= N_RANGE */
            /*
             * out_coord is bounded in [nearest, nearest+1], so we check
             * which of the two input coordinates is closest.
             */
            int write_incr = 0;
            if (fabs(out_coord - (input_start + (nearest+1)*input_spacing)) <
                fabs(out_coord - (input_start + (nearest)*input_spacing)))
            {
                write_incr = 1;
            }
            nearest = nearest + write_incr;
            rmin = nearest - PFA_N_TSINC_POINTS_PER_SIDE;
            if (rmin < 0) { rmin = 0; }
            rmax = nearest + PFA_N_TSINC_POINTS_PER_SIDE;
            if (rmax >= N_RANGE) { rmax = N_RANGE-1; }

            window_offset = 0;
            if (nearest - PFA_N_TSINC_POINTS_PER_SIDE < 0)
            {
                window_offset = PFA_N_TSINC_POINTS_PER_SIDE - nearest;
            }

            // _complex accum_arr[NUMBER_RANGES_FOR_EACH_AIE];
            #pragma clang loop vectorize(disable)
            for (k = 0; k < NUMBER_RANGES_FOR_EACH_AIE; ++k)
            {
                win_val[k] = window[window_offset+(k-rmin)];
            }
            accum.re = 0.0f;
            accum.im = 0.0f;
            #pragma clang loop vectorize(enable)
            for (k = 0; k < NUMBER_RANGES_FOR_EACH_AIE; ++k)
            {
                 //win_val = window[window_offset+(k-rmin)];
                sinc_arg = (out_coord - (input_start+k*input_spacing)) * input_spacing_inv;
                sinc_val = sinc(sinc_arg);
                data = data_i_0;
                if (k < 4) { // each data_i has data for 4 ranges
					// data = data_i_0;
										data_index = (k * N_PULSES) + p;
								} else {
					// data = data_i_1;
										data_index = ((k-4) * N_PULSES) + p;
								}
                if (write_zero) {
                    /* to not accessing wrong memory */
                    data_index = 0;
                }
                _complex new_accum;
                // new_accum =  data[data_index] * (sinc_val * win_val);
                float one_time_mul;
                 one_time_mul = sinc_val * win_val[k];
                new_accum.re =  data[data_index].re * one_time_mul;
                new_accum.im =  data[data_index].im * one_time_mul;

                if (k<rmin || k> rmax) {
                    new_accum.re = 0.0f;
                    new_accum.im = 0.0f;
									}
                accum.re += new_accum.re;
                accum.im += new_accum.im;
                // accum_arr[k] = new_accum;

            }

           g_result[p*16+r].re = accum.re * scale_factor;
           g_result[p*16+r].im = accum.im * scale_factor;

        }
    }
}

/*
 * C89 does not include the family of round() functions, so
 * we include a simple implementation of round().  This version is
 * naive in the sense that it does not work for all inputs and
 * does not honor rounding modes specified by, e.g., fesetround().
 */
static inline int naive_round(float x)
{
	return (int) (x + 0.5f);
}

int find_nearest_range_coord(
    float target_coord,
    float input_coord_start,
    float input_coord_spacing,
    float input_coord_spacing_inv)
{
    /*
     * Test for the target coordinate being out-of-bounds with respect to
     * the input coordinates.
     */
	if (target_coord < input_coord_start ||
	    target_coord >= (input_coord_start + (N_RANGE-1)*input_coord_spacing)) {
		return -1;
	}

	return naive_round((target_coord - input_coord_start) * input_coord_spacing_inv);
}

/**************************************************************************************************/
/* 					top level function					  */
/**************************************************************************************************/
void sar_interp1_top(float * __restrict__ in_data_0,
					// float * __restrict__ in_data_1,
		            float * __restrict__ out_data) {
	Interp1DataIn * __restrict__ interp1datain = (Interp1DataIn * __restrict__)in_data_0;
	_complex * __restrict out_data_complex = (_complex * __restrict__)out_data;
	// Interp1DataIn * __restrict__ interp1datain1 = (Interp1DataIn * __restrict__)in_data_1;
	// Interp1DataOut * __restrict__ interp1dataout = (Interp1DataOut * __restrict__)out_data;
    sar_interp1 (
			// interp1dataout->resampled,
			interp1datain->data_i,
			interp1datain->window,
			interp1datain->input_coords_start,
			interp1datain->input_coords_spacing,
			interp1datain->output_coords,
			out_data_complex
			// interp1datain1->data_i
             );
    //*out_data = 1;
}
