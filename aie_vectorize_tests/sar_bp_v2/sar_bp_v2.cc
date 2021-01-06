
#define N_RANGE 512
#define N_PULSES 512
#define BP_NPIX_X 512
#define BP_NPIX_Y 512

#define NUM_SAMPLES 256
#define RANGE_UPSAMPLE_FACTOR (8)

#define N_RANGE_UPSAMPLED  (N_RANGE * RANGE_UPSAMPLE_FACTOR)
#define NEW_N_RANGE_UPSAMPLED  (N_RANGE_UPSAMPLED/4)
#define NUMBER_PULSES_FOR_EACH_AIE  (8)
/* WHOLE_DATA_SIZE based on structures:
        there are 5 parameters (ku, R0 , dR_inv, dxdy, z0) = 5 float
        + NUMBER_PULSES_FOR_EACH_AIE platpos each one 3 float = 8*3 = 24 float
        + 2*NEW_N_RANGE_UPSAMPLED of complex data each 2 floats = 2* (512*8/4) * 2(floats) = 4K floats
        that will be:
        5 + 24 + 4096 = 4125 float
		whole data is less than 16 KB
*/
#define WHOLE_DATA_SIZE  (5 + (NUMBER_PULSES_FOR_EACH_AIE*3) + (2*NEW_N_RANGE_UPSAMPLED*2))

#include <math.h>
#include "_complex.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define SPEED_OF_LIGHT (3.0e8)
typedef struct _position
{
	float x, y, z;
} position;

// typedef struct  { float re, im; } _complex;

typedef struct {
	float ku;
	float R0;
	float dR_inv;
	float dxdy;
	float z0;
} BpParam;

typedef struct {
	BpParam parameters;
	float platpos_x[NUMBER_PULSES_FOR_EACH_AIE];
	float platpos_y[NUMBER_PULSES_FOR_EACH_AIE];
	float platpos_z[NUMBER_PULSES_FOR_EACH_AIE];
	_complex<float> data[2*NEW_N_RANGE_UPSAMPLED];
} BpData;


const static int boundary[]={1022, 2046, 1022, 2046, 1022, 2046, 1022, 2046};
volatile _complex<float> g_result;
volatile float g_result_re, g_result_im;
/*************************************************************************************************/
/************************************** START OF KERNEL ******************************************/
/*************************************************************************************************/

void inner_function(const BpParam  * __restrict__ param,
		    const float    * __restrict__ platpos_x,
		    const float    * __restrict__ platpos_y,
		    const float    * __restrict__ platpos_z,
		    const _complex<float> * __restrict__ data_0
			) {

	float px = -64.125f;
	float py = -64.125f;
	float ku = param->ku;
	float R0 = param->R0;
	float dR_inv = param->dR_inv;
	_complex<float> zero_complex(0.0f, 0.0f);
	_complex<float> sample, matched_filter, prod;


#pragma clang loop vectorize(enable)
	for (unsigned iy = 0; iy < 16; ++iy){
		py = py + 0.25f;
		px = -64.125f;
#pragma clang loop vectorize(enable)
		for (unsigned ix = 0; ix < 8; ++ix){
			/* complex accum; */
			px = px + 0.25f;
			//result = zero_complex;
			_complex<float> result = zero_complex;
			//_complex<float> tmp_multf;
			_complex<float> *ldata;
			#pragma clang loop vectorize(enable) //interleave_count(2)
			for (unsigned int num_pulse_per_aie=0;
			     num_pulse_per_aie<NUMBER_PULSES_FOR_EACH_AIE;
			     num_pulse_per_aie++){
				const float xdiff_sq = powf((platpos_x[num_pulse_per_aie] - px),2.0f);
				const float ydiff_sq = powf((platpos_y[num_pulse_per_aie] - py),2.0f);
				const float zdiff_sq = powf(platpos_z[num_pulse_per_aie],2.0f);
				const float R = sqrtf(xdiff_sq + ydiff_sq + zdiff_sq);
				const float twice_ku_r = 2.0f * ku * R;
				/* compute the complex exponential for the matched filter */
				matched_filter.real() = cosf(twice_ku_r);//twice_ku_r * 2.0f;//cosf(twice_ku_r);
				matched_filter.imag() = sinf(twice_ku_r);//twice_ku_r * 3.0f;//sinf(twice_ku_r);

				/* convert to the range of bin index */
				const float bin = (R - R0) * dR_inv;
				const int bin_floor = bin;
				bool write_zero = true;
				const _complex<float> * __restrict__ data ;
				int new_bin = (bin_floor % (NEW_N_RANGE_UPSAMPLED)) + (NEW_N_RANGE_UPSAMPLED*num_pulse_per_aie);
				int compare_boundary = new_bin < boundary[num_pulse_per_aie];
				/* interpolation range is [bin_floor, bin_floor+1] */
				if (bin_floor >= 0 && bin_floor <= N_RANGE_UPSAMPLED-2 && compare_boundary) {
						write_zero = false;
				} else {
					/* if bin is out of the range then the data is not valid
						make new_bin zero to prevent accessing wrong memory */
					new_bin = 0;
				}
				data = data_0;

				/* interpolation weight */
				const float w = bin - bin_floor;
				/* linearly interpolate to obtain a sample at bin */
				const float new_w = 1.0f - w;
				ldata = const_cast<_complex<float> *>(&data[new_bin]);
				//tmp_multf = ldata[1] * w;
				sample = ldata[1] * w + ldata[0] * new_w;

				/* scale the interpolated sample by the matched filter */
				prod = matched_filter * sample;
				if (write_zero) {
					prod = zero_complex;
				}

				result += prod;
			}

			g_result_re = result.real();
			g_result_im = result.imag();
		}
 	 }

}

/**************************************************************************************************/
/* 					top level function					  */
/**************************************************************************************************/
void sar_bp_top(float * __restrict__ in_data_0,
                float * __restrict__ out_data) {

	BpData * __restrict__ bpdata = (BpData * __restrict__)in_data_0;

	inner_function (&bpdata->parameters,
			bpdata->platpos_x,
			bpdata->platpos_y,
			bpdata->platpos_z,
			bpdata->data
			);
	*out_data = 1;
}
