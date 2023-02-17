#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "_complex.h"


#define N_RANGE 128
#define N_PULSES 128
#define BP_NPIX_X 256
#define BP_NPIX_Y 256
#define PFA_NOUT_RANGE 128
#define PFA_NOUT_AZIMUTH 128
#define T_PFA (13)
#define NUMBER_PULSES_FOR_EACH_AIE (16)
#define NUMBER_RANGES_FOR_EACH_AIE (16)
//for interp2 is 0.048926f
#define FABS_OUTPUT_COORDS (0.048926f)
// 1 AIE provides all the input data
// that has the required data for 16 pulses (NUMBER_PULSES_FOR_EACH_AIE), including:
//      data_i          16pulse*128 (2K)complex = 4K float   = 16KB
//      window                                  128 float   = 512B
//      input_coord    16pulse*128              (2K)float   = 8KB
//      output_coords                           128 float   = 512B

//size of data_i (data_i is complex)
#define INPUT_DATA_I_SIZE NUMBER_PULSES_FOR_EACH_AIE*PFA_NOUT_RANGE
//size of input_coord (input_coord is float)
#define INPUT_COORDS_SIZE NUMBER_PULSES_FOR_EACH_AIE*PFA_NOUT_RANGE
# define PI		3.14159265358979323846f

// typedef struct  { float re, im; } _complex;

typedef struct {
	_complex<float> data_i [INPUT_DATA_I_SIZE];
	float window [PFA_NOUT_RANGE];
	float input_coord[INPUT_COORDS_SIZE];
	float output_coords[PFA_NOUT_RANGE];
} Interp2DataIn;

typedef struct {
	_complex<float> resampled[NUMBER_PULSES_FOR_EACH_AIE*PFA_NOUT_RANGE];
} Interp2DataOut;

static inline int find_nearest_azimuth_coord(
    float target_coord,
    const float *input_coord);

static inline float sinc(float x)
{
    if (x == 0.0f)
    {
        return 1.0f;
    }
    else
    {
        /*
         * C89 does not support sinf(), but we would use it here if it
         * were supported.
         */
        const float arg = PI * x;
        return (float) sinf(arg) / arg;
    }
}

// volatile float g_resampled_re, g_resampled_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/

void sar_interp2(
    // _complex        * __restrict__ resampled,
    const _complex<float>  * __restrict__ data,
    const float     * __restrict__ window,
	const float     * __restrict__ input_coord,
	const float     * __restrict__ output_coords,
    _complex<float> * __restrict__ g_result ) {
    int p, r, k, pmin, pmax, window_offset, data_index, input_coords_index;
    _complex<float>  result;
    _complex<float> zero(0.0f, 0.0f);
    float sinc_arg, sinc_val, win_val;
    float input_spacing_avg, input_spacing_avg_inv, scale_factor;

    const int PFA_N_TSINC_POINTS_PER_SIDE = (T_PFA - 1.0f)/2.0f;

    // for (r = 0; r < NUMBER_RANGES_FOR_EACH_AIE; ++r)
    for (r = 0; r < 8; ++r)
    {
        int new_r = r % NUMBER_RANGES_FOR_EACH_AIE;
        // for image size 256x256, and pulsxrange of 128x128
        // each input_coords_ has data for 16 ranges
        input_coords_index = (new_r * N_PULSES);
        input_spacing_avg = 0.0f;
        #pragma clang loop vectorize(enable)
        // for (p = 0; p < 8; ++p)
        for (p = 0; p < N_PULSES-1; ++p)
        {
            input_spacing_avg += fabs(input_coord[input_coords_index + p + 1] - input_coord[input_coords_index + p]);
        }
        input_spacing_avg /= (N_PULSES-1);
        input_spacing_avg_inv = 1.0f / input_spacing_avg;
        scale_factor = fabs(output_coords[1] - output_coords[0]) * input_spacing_avg_inv;
        // for (p = 0; p < PFA_NOUT_AZIMUTH; ++p)
        for (p = 0; p < 8; ++p)
        {
            bool write_zero = false;
            const float out_coord = output_coords[p];
            // after converting input_coord[r][p] to 1 dimention
            // now to send the address of the second dimention (input_coord[r])
            // is the same as  the address of one dimention r*256 ( &(input_coord[r*N_PULSES]))

            int nearest = find_nearest_azimuth_coord(output_coords[p], &(input_coord[r*N_PULSES]));
            if (nearest < 0)
            {
                write_zero = true;
                // resampled[p][r].re = 0.0f;
                // resampled[p][r].im = 0.0f;
                //g_resampled_re = 0.0f;
                 g_result[r*8 + p] = zero;
                 continue;
                /* to not accessing wrong memory */
                input_coords_index = 0;
                nearest = 0;

			}

            /*
             * out_coord is bounded in [nearest, nearest+1], so we check
             * which of the two input coordinates is closest.
             */
            int write_incr = 0;
            if (fabs(out_coord-input_coord[input_coords_index + nearest + 1]) <
                fabs(out_coord-input_coord[input_coords_index + nearest]))
            {
                write_incr = 1;
            }
            nearest = nearest + write_incr;
            pmin = nearest - PFA_N_TSINC_POINTS_PER_SIDE;
            if (pmin < 0) { pmin = 0; }
            pmax = nearest + PFA_N_TSINC_POINTS_PER_SIDE;
            if (pmax >= N_PULSES) { pmax = N_PULSES-1; }

            window_offset = 0;
            if (nearest - PFA_N_TSINC_POINTS_PER_SIDE < 0)
            {
                window_offset = PFA_N_TSINC_POINTS_PER_SIDE - nearest;
            }
            _complex<float> accum(0.0f,0.0f);
            #pragma clang loop vectorize(enable)
            for (k = 0; k < NUMBER_PULSES_FOR_EACH_AIE; ++k)
            {
                // for image size 256x256, and pulsxrange of 128x128
                // each data_i_ has data for 16 pulses
                data_index = (k * N_RANGE) + r;
                if (write_zero) {
                    /* to not accessing wrong memory */
                    data_index = 0;
                }
                win_val = (float)window[window_offset+(k-pmin)];
                sinc_arg = (out_coord - input_coord[input_coords_index + k]) * input_spacing_avg_inv;
                sinc_val = sinc(sinc_arg);
                // accum.re += sinc_val * win_val * data[(k * N_RANGE) + r].re;
                // accum.im += sinc_val * win_val * data[(k * N_RANGE) + r].im;
                 _complex<float> new_accum;
                 float one_time_mul;
                 one_time_mul = sinc_val * win_val;
                // new_accum.re = data[data_index].re * one_time_mul;
                // new_accum.im = data[data_index].im * one_time_mul;
                new_accum = data[data_index] * one_time_mul;
                bool update = k<pmin || k> pmax;
                if (update) {
					      new_accum = zero;
				}
                accum += new_accum;

            }
            result = accum * scale_factor;
            // result.re = accum.re * scale_factor;
            // result.im = accum.im * scale_factor;
        //    resampled[p][r].re = scale_factor * accum.re;
        //    resampled[p][r].im = scale_factor * accum.im;
           if (write_zero) {
                // resampled[p][r] = zero_complex;
               result = zero;
            }
            //g_resampled_re = result.re;
            g_result[r*8 + p] = result;
        }

    }
}

static inline int find_nearest_azimuth_coord(
    float target_coord,
    const float *input_coord)
{
    int left_ind, right_ind, mid_ind;
    float left_val, right_val, mid_val;

    /*
     * We assume for simplicity that the input coordinates are
     * monotonically increasing.
     */
    // assert(PFA_NOUT_RANGE > 1 && input_coord[1] > input_coord[0]);

    left_ind = 0.0f;
    right_ind = PFA_NOUT_RANGE-1;
    mid_ind = (left_ind+right_ind)/2;
    left_val = input_coord[left_ind];
    right_val = input_coord[right_ind];
    mid_val = input_coord[mid_ind];
    bool sel1 = target_coord < left_val;
    bool sel2 = target_coord > right_val;
    bool sel = sel1|| sel2;
    if (sel)
    {
        return -1;
    }

    while (right_ind - left_ind > 1)
    {
        if (target_coord <= mid_val)
        {
            right_ind = mid_ind;
            right_val = mid_val;
        }
        else
        {
            left_ind = mid_ind;
            left_val = mid_val;
        }
        mid_ind = (left_ind+right_ind)/2;
        mid_val = input_coord[mid_ind];
    }

    return mid_ind;
}
/**********************************************************/
/* 					top level function					  */
/*********************************************************/
void sar_interp2_top(float * __restrict__ in_data_0,
		            float * __restrict__ out_data) {
	Interp2DataIn * __restrict__ interp2datain = (Interp2DataIn * __restrict__)in_data_0;
    _complex<float> * __restrict out_data_complex = (_complex<float> * __restrict__)out_data;
	// Interp2DataOut * __restrict__ interp2dataout = (Interp2DataOut * __restrict__)out_data;
    sar_interp2 (
			// interp2dataout->resampled,
			interp2datain->data_i,
			interp2datain->window,
			interp2datain->input_coord,
			interp2datain->output_coords,
            out_data_complex);

}
