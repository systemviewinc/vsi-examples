
#define N_RANGE 128
#define N_PULSES 128
#define BP_NPIX_X 256
#define BP_NPIX_Y 256

#define NUM_SAMPLES 256
#define RANGE_UPSAMPLE_FACTOR (8)

#define N_RANGE_UPSAMPLED  (N_RANGE * RANGE_UPSAMPLE_FACTOR)
#define N_RANGE_UPSAMPLED_256x256  (N_RANGE * RANGE_UPSAMPLE_FACTOR *2)
// #define NEW_N_RANGE_UPSAMPLED  (N_RANGE * 2)
#define NEW_N_RANGE_UPSAMPLED  (N_RANGE_UPSAMPLED/4)
#define NUMBER_PULSES_FOR_EACH_AIE  (8)
/* WHOLE_DATA_SIZE:

        there are 5 parameters (ku, R0 , dR_inv, dxdy, z0)
        + NUMBER_PULSES_FOR_EACH_AIE platpos each one 3 float = 8*3 = 24 float
        + NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED of complex data each 2 floats
        that will be:
        	5 + 24 + (8*(128*2)*2) =
        	5 + 24 + 4096 = 4125 float data coming from 1 AIE to do the computation
		each AIE provide 4K float + 24 + 5 float = more than 16KB data
		1 AIE provides all the input data
		complex data is NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED of complex data
		that is: 8*(128*2) 
		each AIE has the required data for 8 pulses (NUMBER_PULSES_FOR_EACH_AIE), including:
			there are 5 parameters (ku, R0 , dR_inv, dxdy, z0)
        	+ NUMBER_PULSES_FOR_EACH_AIE platpos each one 3 float = 8*3 = 24 float
        	+ NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED of complex data each 2 floats
        that will be:
			5 + 24 + (8*(128*2)*2) =
			5 + 24 + 4096 = 4125 float
*/
#define WHOLE_DATA_SIZE  (8 + (24) + (NUMBER_PULSES_FOR_EACH_AIE*NEW_N_RANGE_UPSAMPLED))

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
  float a0;
  float a1; // sending 4 zeros for alignment 
  float a2;
  float a3;
} BpParam;

typedef struct {
	BpParam parameters;
	float platpos_x[NUMBER_PULSES_FOR_EACH_AIE];
	float platpos_y[NUMBER_PULSES_FOR_EACH_AIE];
	float platpos_z[NUMBER_PULSES_FOR_EACH_AIE];
	// each data has information for 8 pulses
	_complex data[8*NEW_N_RANGE_UPSAMPLED];
} BpData;


/* complex multiplication */
static inline _complex cmult(_complex lhs, _complex rhs)
{
	_complex prod;
	prod.re = lhs.re * rhs.re - lhs.im * rhs.im;
	prod.im = lhs.re * rhs.im + lhs.im * rhs.re;
	return prod;
}
/* boundary for 128x128 that we have all 8 pulses data is  
	bin < NEW_N_RANGE_UPSAMPLED -2
	for new_bin (that 2D array becomes 1D array) it will be:
	new_bin < (NEW_N_RANGE_UPSAMPLED * (num_pulse_per_aie%8 + 1)) -2
	new_bin < (NEW_N_RANGE_UPSAMPLED * (num_pulse_per_aie + 1)) -2
	
*/
const static int boundary[]={254, 510, 766, 1022, 1278, 1534, 1790, 2046};

//volatile complex g_result;
//float g_result_re, g_result_im;
/**************************************************************************************************/
/************************************ START OF KERNEL ***************************************/
/**************************************************************************************************/

void inner_function(const BpParam  * __restrict__ param,
		    const float    * __restrict__ platpos_x,
		    const float    * __restrict__ platpos_y,
		    const float    * __restrict__ platpos_z,
		    const _complex * __restrict__ data,
			_complex * __restrict__ g_result) {

	float px = -64.125f;
	float py = -64.125f;
	float ku = param->ku;
	float R0 = param->R0;
	float dR_inv = param->dR_inv;
	for (unsigned iy = 0; iy < 16; ++iy){
		py = py + 0.25f;
		px = -64.125f;
		for (unsigned ix = 0; ix < 8; ++ix){
			_complex result;
			result.re = 0.0f;		
			result.im = 0.0f;		
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
				//since data is converted from 2 dimention array to one dimention 
				// the bin also should be adjusted for 1 dimention array:
				// + (NEW_N_RANGE_UPSAMPLED*num_pulse_per_aie)
				int new_bin = (bin_floor % NEW_N_RANGE_UPSAMPLED) + (NEW_N_RANGE_UPSAMPLED*num_pulse_per_aie);
				int compare_boundary = new_bin < boundary[num_pulse_per_aie];
				/* interpolation range is [bin_floor, bin_floor+1] */	
				/* for image size 256x256 the N_RANGE_UPSAMPLED is 1024 but
				  the bin_floor could still be any number less than 4096 so
				  the condition changed to (N_RANGE_UPSAMPLED*4)*/				
				if (bin_floor >= 0 && bin_floor <= (N_RANGE_UPSAMPLED*4)-2 && compare_boundary) {
						write_zero = false;
				} else {
					/* if bin is out of the range then the data is not valid
						make new_bin zero to prevent accessing wrong memory */
					new_bin = 0;
				}
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
			}
			g_result[iy*16 + ix].re = result.re;
			g_result[iy*16 + ix].im = result.im;
		}
 	 }
}

/**************************************************************************************************/
/* 					top level function					  */
/**************************************************************************************************/
void sar_bp_top(float * __restrict__ in_data,
		float * __restrict__ out_data) {
	BpData * __restrict__ bpdata = (BpData * __restrict__)in_data;
	_complex * __restrict out_data_complex = (_complex * __restrict__)out_data;
	
	inner_function (&bpdata->parameters,
			bpdata->platpos_x,
			bpdata->platpos_y,
			bpdata->platpos_z,
			bpdata->data,
			out_data_complex);
}
