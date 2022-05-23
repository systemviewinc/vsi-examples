#ifndef __KERNELS_H__
#define __KERNELS_H__




#define SNAPSHOT_SIZE 768   //The size is in float = 384 cfloat
#define COVARIANCE_SIZE 288  //The size is in float = 144 cfloat
#define CHOLESKY_FACTOR_SIZE 288 //The size is in float = 144 cfloat
// #define STEERING_VECTOR_SIZE 384  //The size is in float = 192 cfloat
#define STEERING_VECTOR_SIZE 24*16  //The size is in float = 12 cfloat
#define ADAPTIVE_WEIGHTS_SIZE 24  //The size is in float = 12 cfloat
#define FINAL_OUT_DATA_SIZE 64  //The size is in float = 32 cfloat
#ifndef __VSI_AUTOVEC__
float snapshot_in_buffer[SNAPSHOT_SIZE];
float covariance_buffer[COVARIANCE_SIZE]={};
// float covariance_buffer[COVARIANCE_SIZE];
float cholesky_factor_buffer[CHOLESKY_FACTOR_SIZE];
// float cholesky_factor_buffer[CHOLESKY_FACTOR_SIZE];
float steering_vector_in_buffer[STEERING_VECTOR_SIZE];
float adaptive_weights_out_buffer[ADAPTIVE_WEIGHTS_SIZE];
float final_data_out_buffer[FINAL_OUT_DATA_SIZE];
#endif 

			
#endif /**********__KERNELS_H__**********/