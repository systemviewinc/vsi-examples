#include "fft_control.h"

struct t_size t_size_array [] = {
	{0x03, 		8},
	{0x04, 	       16},
	{0x05, 	       32},
	{0x06, 	       64},
	{0x07,	      128},
	{0x08,	      256},
	{0x09,	      512},
	{0x0a,	     1024},
	{0x0b,	     2048},
	{0x0c,	     4096},
	{0x0d,	     8192},
	{0x0e,	    16384},
	{0x0f,	    32768},
	{0x10,	    65536}
};
#define T_SIZE_SIZE sizeof(t_size_array)/sizeof(t_size)


void send_control_fft (hls::stream<fft_control_s> &control, int b_size)
{
	struct fft_control_s fcs;
	int i;
	for (i = 0 ; i < T_SIZE_SIZE ; i++) {
		if (t_size_array[i].t_bytes ==  b_size) break;
	}
	if (i == T_SIZE_SIZE) fcs.data = 0x10;
	else fcs.data = t_size_array[i].t_size;
	control.write(fcs);
}

void send_wave_fft (hls::stream<fft_data_s> &fft_ds, hls::stream<int> &cont, FILE *wavf, int s_size, int p_size)
{
	int ps_size = 0;
	struct fft_data_s fd;
	while (!feof(wavf)) {
		ps_size++;
		if (s_size == 8) {
			unsigned char d;
			fread(&d,sizeof(d),1,wavf);
			fd.data.re = (float)d;
			fd.data.im = 0.0 ;
		} else if (s_size == 16) {
			unsigned short d;
			fread(&d,sizeof(d),1,wavf);
			fd.data.re = (float)d;
			fd.data.im = 0.0 ;
		} else if (s_size == 32) {
			unsigned int d;
			fread(&d,sizeof(d),1,wavf);
			fd.data.re = (float)d;
			fd.data.im = 0.0 ;
		}
		fd.last = (ps_size == p_size);
		fft_ds.write(fd); // send it
		// if end of packet reached
		if (fd.last) {
			printf("sent packet .. waiting for response ..%d:%d\n",ps_size,p_size);
			int w = cont.read(); // wait for data to be processed
			ps_size = 0;
		}
		if (feof(wavf)) break;
	}
	if (ps_size == p_size) return; // done
	for (; ps_size != p_size; ps_size++) {
		fd.data.re = fd.data.im = 0.0;
		fd.last = 0;
		fft_ds.write(fd); // send it		
	}
	fd.data.re = fd.data.im = 0.0;
	fd.last = 1;
	fft_ds.write(fd); // send it			
}

void recv_fft_data (hls::stream<fft_data_s> &fft_ds, hls::stream<int> &cont)
{
	while (1) {
		// gather data
		fft_data_s data_s = fft_ds.read();
		if (data_s.last) {
			printf(" Got a response packet\n");
			cont.write(1); // let the send continue
		}
	}
}

int fft_main(hls::stream<fft_control_s> &control, hls::stream<fft_data_s> &fft_ds, hls::stream<int> &cont){
	wav_hdr wavHeader;
	wav_gen_hdr wavChdr;
	FILE *wavFile;
	int headerSize = sizeof(wav_hdr),filelength = 0;
	bool data_chunk = false;
	unsigned int data_size ;
	std::string input;
	std::string answer;
	
	const char* filePath;
	
	std::cout << "Pick wav file from the Windows Media File: ";
	std::cin >> input;
	std::cin.get();
	
	std::cout << std::endl;
	
	std::string path = input + ".wav";
	filePath = path.c_str();
	
	wavFile = fopen( filePath , "r" );
	
	if(wavFile == NULL){
		printf("Can not able to open wave file %s\n",filePath);
		exit(EXIT_FAILURE);
	}
	
	fread(&wavHeader,headerSize,1,wavFile);
	
	std::cout << "RIFF header                :"
		  << wavHeader.RIFF[0] 
		  << wavHeader.RIFF[1] 
		  << wavHeader.RIFF[2] 
		  << wavHeader.RIFF[3] << std::endl;
	
	std::cout << "WAVE header                :"
		  << wavHeader.WAVE[0] 
		  << wavHeader.WAVE[1] 
		  << wavHeader.WAVE[2] 
		  << wavHeader.WAVE[3] 
		  << std::endl;
	
	std::cout << "FMT                        :"
		  << wavHeader.fmt[0] 
		  << wavHeader.fmt[1] 
		  << wavHeader.fmt[2] 
		  << wavHeader.fmt[3] 
		  << std::endl;
	
	std::cout << "Data size                  :"
		  << wavHeader.ChunkSize << std::endl;
	
	// Display the sampling Rate form the header
	std::cout << "Sampling Rate              :" << wavHeader.SamplesPerSec << std::endl;
	std::cout << "Number of bits used        :" << wavHeader.bitsPerSample << std::endl;
	std::cout << "Number of channels         :" << wavHeader.NumOfChan << std::endl;
	std::cout << "Number of bytes per second :" << wavHeader.bytesPerSec << std::endl;
	std::cout << "Data length                :" << wavHeader.Subchunk2Size << std::endl;
	std::cout << "Audio Format               :" << wavHeader.AudioFormat << std::endl;
	// Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
	
	
	std::cout << "Block align                :" << wavHeader.blockAlign << std::endl;
	std::cout << "Header Size                :" << headerSize << std::endl;
	std::cout << "Data string                :" << wavHeader.Subchunk2ID[0] 
		  << wavHeader.Subchunk2ID[1]
		  << wavHeader.Subchunk2ID[2] 
		  << wavHeader.Subchunk2ID[3] 
		  << std::endl;

	// look for the data chunk 
	if (wavHeader.Subchunk2ID[0] != 'd' ||
	    wavHeader.Subchunk2ID[1] != 'a' ||
	    wavHeader.Subchunk2ID[2] != 't' ||
	    wavHeader.Subchunk2ID[3] != 'a') {
		int fs = fseek(wavFile, (long int)wavHeader.Subchunk2Size, SEEK_CUR);
		if (fs < 0) {
			perror("fseek returned error ");
			exit(-1);
		}
		std::cout << "Not Data chunk skipped " << wavHeader.Subchunk2Size << " bytes " << std::endl;
		while (1) {
			int cb = fread(&wavChdr,sizeof(wavChdr),1,wavFile);
			if (wavChdr.CTYPE[0] == 'd' &&
			    wavChdr.CTYPE[1] == 'a' &&
			    wavChdr.CTYPE[2] == 't' &&
			    wavChdr.CTYPE[3] == 'a') {
				data_size = wavChdr.ChunkSize;
				data_chunk = true;
				break;
			}
			fseek(wavFile, wavChdr.ChunkSize, SEEK_CUR);
			std::cout << "Not Data chunk skipped " << wavChdr.ChunkSize << " bytes " << std::endl;
		}
	}
	//send_control_fft (control,1024);
	send_wave_fft (fft_ds, cont, wavFile, wavHeader.bitsPerSample, 1024);
} 


