
#define N_RANGE 512
#define N_PULSES 512
#define BP_NPIX_X 512
#define BP_NPIX_Y 512

#define NUM_SAMPLES 256
#define RANGE_UPSAMPLE_FACTOR (8)

#define N_RANGE_UPSAMPLED  (N_RANGE * RANGE_UPSAMPLE_FACTOR)
#define NEW_N_RANGE_UPSAMPLED  (N_RANGE_UPSAMPLED/4)
#define NUMBER_PULSES_FOR_EACH_AIE  (8)
/* WHOLE_DATA_SIZE:
        there are 8 parameters (including: ku, R0 , dR,...)
        + N_PULSES platpos each one 3 float
        + NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED of complex data each 2 floats
        that will be:
        8 + (N_PULSE*3) + (NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED*2) =
        8 + 1536 + 4096 = 5640 float data = 22kB data
*/
#define WHOLE_DATA_SIZE  (8 + (N_PULSES*3) + (NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED*2))

#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define SPEED_OF_LIGHT (3.0e8)
typedef struct _position
{
	float x, y, z;
} position;

typedef struct  { float re, im; } _complex;

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
	_complex data[2*NEW_N_RANGE_UPSAMPLED];
} BpData;


/* complex multiplication */
static inline _complex cmult(_complex lhs, _complex rhs)
{
	_complex prod;
	prod.re = lhs.re * rhs.re - lhs.im * rhs.im;
	prod.im = lhs.re * rhs.im + lhs.im * rhs.re;
	return prod;
}

const static int boundary[]={1022, 2046, 1022, 2046, 1022, 2046, 1022, 2046};

//volatile complex g_result;
volatile float g_result_re, g_result_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/

void inner_function(const BpParam  * __restrict__ param,
		    const float    * __restrict__ platpos_x,
		    const float    * __restrict__ platpos_y,
		    const float    * __restrict__ platpos_z,
		    const _complex * __restrict__ data_0) {

	float px = -64.125f;
	float py = -64.125f;
	float ku = param->ku;
	float R0 = param->R0;
	float dR_inv = param->dR_inv;
	for (unsigned iy = 0; iy < 16; ++iy){
		py = py + 0.25f;
		px = -64.125f;
		for (unsigned ix = 0; ix < 8; ++ix){
			// float res_re[NUMBER_PULSES_FOR_EACH_AIE];			
			// float res_im[NUMBER_PULSES_FOR_EACH_AIE];	
			_complex result;
			result.re = 0.0f;		
			result.im = 0.0f;		
			/* complex accum; */
			px = px + 0.25f;
#pragma clang loop vectorize(enable)
			for (unsigned int num_pulse_per_aie=0;
			     num_pulse_per_aie<NUMBER_PULSES_FOR_EACH_AIE;
			     num_pulse_per_aie++){
				const float xdiff_sq = powf((platpos_x[num_pulse_per_aie] - px),2.0f);
				const float ydiff_sq = powf((platpos_y[num_pulse_per_aie] - py),2.0f);
				const float zdiff_sq = powf(platpos_z[num_pulse_per_aie],2.0f);
				const float R = sqrtf(xdiff_sq + ydiff_sq + zdiff_sq);
				const float twice_ku_r = 2.0f * ku * R;
				_complex sample, matched_filter, prod;
				/* compute the complex exponential for the matched filter */
				matched_filter.re = cosf(twice_ku_r);
				matched_filter.im = sinf(twice_ku_r);
				/* convert to a range bin index */
				const float bin = (R - R0) * dR_inv;				
				const int bin_floor = bin;
				bool write_zero = true;
				const _complex * __restrict__ data ;
				int new_bin = (bin_floor % NEW_N_RANGE_UPSAMPLED) + (NEW_N_RANGE_UPSAMPLED*num_pulse_per_aie);
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
				// if (num_pulse_per_aie < 2){
				// 	data = data_0;
				// } else if (num_pulse_per_aie < 4){
				// 	data = data_1;
				// } else if (num_pulse_per_aie < 6){
				// 	data = data_2;
				// } else {
				// 	data = data_3;
				// }
				/* interpolation weight */
				const float w = bin - bin_floor;
				/* linearly interpolate to obtain a sample at bin */
				const float new_w = 1.0f - w;
				const _complex *ldata = &data[new_bin];
				sample.re = new_w*ldata[0].re + w*ldata[1].re;
				sample.im = new_w*ldata[0].im + w*ldata[1].im;
				
				/* scale the interpolated sample by the matched filter */
				prod = cmult(sample, matched_filter);
				if (write_zero) {
					prod.re = 0.0f;
					prod.im = 0.0f;
				}
				result.re += prod.re;
				result.im += prod.im;
				// res_re[num_pulse_per_aie] = prod.re;
				// res_im[num_pulse_per_aie] = prod.im;
			       
			}
			// float resr_re, resr_im;;
			// resr_re = resr_im = 0.0f;
			// for (int i = 0 ; i < NUMBER_PULSES_FOR_EACH_AIE ; i++) {
			// 	resr_re += res_re[i];
			// }
			// for (int i = 0 ; i < NUMBER_PULSES_FOR_EACH_AIE ; i++) {
			// 	resr_im += res_im[i];
			// }
			// g_result_re = resr_re;
			// g_result_im = resr_im;
			g_result_re = result.re;
			g_result_im = result.im;
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
