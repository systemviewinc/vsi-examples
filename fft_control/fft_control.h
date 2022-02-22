#ifndef FFT_CONTROL_H
#define FFT_CONTROL_H

#include <iostream>
#include <string>
#include <fstream>
#include <hls_stream.h>
#include <ap_utils.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>

struct t_size {
	ap_uint<5> t_size;
	int	   t_bytes;
};


typedef struct _fft_data {
	float re;	// real part
	float im;	// imaginary part
	static const int width = 2*sizeof(float)*8; // width in bits
} fft_data;

struct fft_data_s {
	fft_data	data;	
	ap_uint<1>	last;	// end of packet
};

struct fft_control_s {
	ap_uint<24>    data;
};


/* Wave file .WAV file format */

typedef struct  WAV_HEADER {
	char                RIFF[4];        // RIFF Header      Magic header
	unsigned int        ChunkSize;      // RIFF Chunk Size  
	char                WAVE[4];        // WAVE Header      
	char                fmt[4];         // FMT header       
	unsigned int        Subchunk1Size;  // Size of the fmt chunk                                
	unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
	unsigned short      NumOfChan;      // Number of channels 1=Mono 2=Sterio                   
	unsigned int        SamplesPerSec;  // Sampling Frequency in Hz                             
	unsigned int        bytesPerSec;    // bytes per second 
	unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
	unsigned short      bitsPerSample;  // Number of bits per sample      
	char                Subchunk2ID[4]; // "data"  string   
	unsigned int        Subchunk2Size;  // Sampled data length    

} wav_hdr; 

typedef struct {
	char 		    CTYPE[4]; 	// Chuck Type "data" is ours
	unsigned int	    ChunkSize;
} wav_gen_hdr;

#endif
