#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "_complex.h"

#define N_RANGE 512
#define N_PULSES 512
#define BP_NPIX_X 512
#define BP_NPIX_Y 512
#define PFA_NOUT_RANGE 512
#define PFA_NOUT_AZIMUTH 512
#define T_PFA (13)
#define NUMBER_PULSES_FOR_EACH_AIE (8)
#define NUMBER_RANGES_FOR_EACH_AIE (8)
//for interp2 is 0.048926f
#define FABS_OUTPUT_COORDS (0.048926f)
// 2 AIEs provides all the input data
// each AIE has the required data for 4 pulses (NUMBER_PULSES_FOR_EACH_AIE/2), including:
//      data_i          4pulse*512 (2K)complex = 4K float   = 16KB
//      window                                  512 float   = 2KB
//      input_coord    4pulse*512              (2K)float   = 8KB
//      output_coords                           512 float   = 2KB

//size of data_i (data_i is complex)
#define INPUT_DATA_I_SIZE NUMBER_PULSES_FOR_EACH_AIE*PFA_NOUT_RANGE/2
//size of input_coord (input_coord is float)
#define INPUT_COORDS_SIZE NUMBER_PULSES_FOR_EACH_AIE*PFA_NOUT_RANGE/2
# define PI		3.14159265358979323846f

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
        return (float) sin(arg) / arg;
    }
}

volatile float g_resampled_re, g_resampled_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/

void sar_interp2(
    // _complex<float>        * __restrict__ resampled,
    const _complex<float>  * __restrict__ data_i_0,
    const float     * __restrict__ window,
	const float     * __restrict__ input_coords_0,
	const float     * __restrict__ output_coords
    // const _complex<float>  * __restrict__ data_i_1,
	// const float     * __restrict__ input_coords_1
) {
    int p, r, k, pmin, pmax, window_offset, data_index, input_coords_index;
    _complex<float> zero_complex(0.0f, 0.0f), result;
    float sinc_arg, sinc_val, win_val[NUMBER_PULSES_FOR_EACH_AIE];
    float input_spacing_avg, input_spacing_avg_inv, scale_factor;
    _complex<float> accum_arr[NUMBER_PULSES_FOR_EACH_AIE];
    const _complex<float> * __restrict__ data ;
    const float * __restrict__ input_coord ;

    const int PFA_N_TSINC_POINTS_PER_SIDE = (T_PFA - 1.0f)/2.0f;


    // for (r = 0; r < NUMBER_RANGES_FOR_EACH_AIE; ++r)
    for (r = 0; r < 8; ++r)
    {
        _complex<float> accum;
        input_coord = input_coords_0;
        int new_r = r % NUMBER_RANGES_FOR_EACH_AIE;
        if (new_r < 4) { // each input_coords_ has data for 4 ranges
            // input_coord = input_coords_0;
            input_coords_index = (new_r * N_PULSES);
        } else {
            // input_coord = input_coords_1;
            input_coords_index = ((new_r-4) * N_PULSES);
        }
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
        bool write_zero = false;
        // for (p = 0; p < PFA_NOUT_AZIMUTH; ++p)
        for (p = 0; p < 8; ++p)
        {
            const float out_coord = output_coords[p];
            // after converting input_coord[r][p] to 1 dimention
            // now to send the address of the second dimention (input_coord[r])
            // is the same as  the address of one dimention r*512 ( &(input_coord[r*N_PULSES]))
            
            int nearest = find_nearest_azimuth_coord(output_coords[p], &(input_coord[r*N_PULSES]));
            if (nearest < 0)
            {
                write_zero = true;
                // resampled[p][r].re = 0.0f;
                // resampled[p][r].im = 0.0f;
                // continue;
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
            accum = zero_complex;
            #pragma clang loop vectorize(disable) 
            for (k = 0; k < NUMBER_PULSES_FOR_EACH_AIE; ++k)
            {
                win_val[k] = (float)window[window_offset+(k-pmin)];
            }
            #pragma clang loop vectorize(enable) 
            for (k = 0; k < NUMBER_PULSES_FOR_EACH_AIE; ++k)
            {
                data = data_i_0;
                if (k < 4){
                    // data = data_i_0;
                    data_index = (k * N_RANGE) + r;
                } else {
                    // data = data_i_1;
                    data_index = ((k-4) * N_RANGE) + r;
                }
                if (write_zero) {
                    /* to not accessing wrong memory */
                    data_index = 0;
                }
                // win_val = (float)window[window_offset+(k-pmin)];
                sinc_arg = (out_coord - input_coord[input_coords_index + k]) * input_spacing_avg_inv;
                sinc_val = sinc(sinc_arg);
                 _complex<float> new_accum;
                new_accum = data[data_index] * (sinc_val * win_val[k]);
                bool update = k<pmin || k> pmax;
                if (update) {
					new_accum = zero_complex;
				}
                accum +=new_accum;
            }
            

            result = accum * scale_factor;
           if (write_zero) {
                result = zero_complex;
            }
            g_resampled_re = result.real();
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
					// float * __restrict__ in_data_1,
		            float * __restrict__ out_data) {
	Interp2DataIn * __restrict__ interp2datain = (Interp2DataIn * __restrict__)in_data_0;
	// Interp2DataIn * __restrict__ interp2datain1 = (Interp2DataIn * __restrict__)in_data_1;
	// Interp2DataOut * __restrict__ interp2dataout = (Interp2DataOut * __restrict__)out_data;
    sar_interp2 (
			// interp2dataout->resampled,
			interp2datain->data_i,
			interp2datain->window,
			interp2datain->input_coord,
			interp2datain->output_coords
			// interp2datain1->data_i,
			// interp2datain1->input_coord
             );
    *out_data = 1;
}

