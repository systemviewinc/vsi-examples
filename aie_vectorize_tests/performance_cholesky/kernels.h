#ifndef __KERNELS_H__
#define __KERNELS_H__

#define N_CHAN (8)
#define N_RANGE (256)
#define TRAINING_BLOCK_SIZE (128)
#define sample_norm (1.0f / TRAINING_BLOCK_SIZE)
#define N_BLOCKS (N_RANGE/TRAINING_BLOCK_SIZE)
#define N_PULSES (512)
#define N_DOP (512)
#define TDOF (1)
#define N_STEERING (1)
#define SV_SIZE (N_CHAN*TDOF*2) // each Steering vector size. Steering vector is array of complex floats (each 2 floats), so total memory size in bytes is SV_SIZE*4

#define SNAPSHOT_SIZE (SV_SIZE*TRAINING_BLOCK_SIZE)   //The size is in float = 384 cfloat
// Snapshot size includes snapshot vectors across all channels in a block
#define COVARIANCE_SIZE ((N_CHAN*TDOF)*(N_CHAN*TDOF))*2  //The size is in float = SV_SIZE^2 cfloat
#define CHOLESKY_FACTOR_SIZE ((N_CHAN*TDOF)*(N_CHAN*TDOF))*2 //The size is 64 cfloat = 64*8 = 512 bytes
#define STEERING_VECTOR_SIZE (N_CHAN*TDOF*2)*N_STEERING  //The size is in float = 12 cfloat
#define ADAPTIVE_WEIGHTS_SIZE (N_CHAN*TDOF*2)  //The size is in float = N_CHAN*TDOF cfloat
// #define FINAL_OUT_DATA_SIZE TRAINING_BLOCK_SIZE*2  //The size is in float = TRAINING_BLOCK_SIZE cfloat
#define FINAL_OUT_DATA_SIZE CHOLESKY_FACTOR_SIZE // the final_data_out_buffer array is from type float and CHOLESKY_FACTOR_SIZE is already in float size

#ifndef __VSI_AUTOVEC__

float snapshot_in_buffer[SNAPSHOT_SIZE];
float covariance_buffer[COVARIANCE_SIZE];
float cholesky_factor_buffer[CHOLESKY_FACTOR_SIZE];

//float steering_vector_in_buffer[STEERING_VECTOR_SIZE];
//float adaptive_weights_out_buffer[ADAPTIVE_WEIGHTS_SIZE];
float final_data_out_buffer[FINAL_OUT_DATA_SIZE];
//#endif 


#endif /**********auto vectorizer **********/
#endif /**********__KERNELS_H__**********/
