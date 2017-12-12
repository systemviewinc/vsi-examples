#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "suputils.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstddef>
#include <syslog.h>
#include <cstring>

using namespace std;
#define __COUNT_DATA__

int datavalidcheck(int attemptID, char * readBuf, 
	char * writeBuf, unsigned int bufSize){
	// compare read and write buffer
	int rc = std::memcmp(writeBuf, readBuf, bufSize);
	
	if(rc){ // branch for different buffers
		cout << "Corupted data!" << endl;
	}
	else // branch for valid r/w
		cout << "Ok";

	cout << " "<< attemptID << endl << flush;
	return rc;
}

void speeddatahnd(unsigned int transID,
	long long int writetime,
	long long int readtime,
	unsigned int bufSize){

	cout << "{memory test} " << dec << " Size :\t" << bufSize << "\tByte #" << transID;
				cout << " ws :\t" << speedCalc(bufSize, writetime) << "\tKb/s ";
				cout << " rs :\t" << speedCalc(bufSize, readtime) << "\tKb/s ";
}

void prinFirstLastHex(char *buf, int size, int num)
{
	if(num > size)
		prinHex(buf, size);
	else{
		prinHex(buf, num);
		cout  << " ... " ;
		prinHex(buf + size - num, num);
		cout << endl;
	}
}



void printHex32_2col(char *buf, char *buf2, int size)
{
	for(int i = 0; i < size; i += 4){
		cout << dec << std::setw(4) << setfill('0') << i/4 << " # ";
		cout << hex << std::setw(8) << setfill('0') << *(int *)(buf + i) << "   " \
			<< hex << std::setw(8) << setfill('0') << *(int *)(buf2 + i);
		
		if(0){
		  if(
			(*(int *)(buf2 + i)) != ((*(int *)(buf2 + i - 4))+1) &&
			((*(int *)(buf2 + i)) != ((*(int *)(buf2 + i - 4))) != 0)

			)
			cout << " ^ ";
		}
		cout << endl;
		if(0)
			if(i & 0x3F) 
				cout << endl;
			else
				cout << " \t ";
	}
	cout << dec;
}

int mempretest(char *buf, int size){
	volatile int firstVal = (int)rand();
	volatile int lastVal = (int)rand();
	*(int *)(buf) = firstVal;
	*(int *)(buf + size - sizeof(int)) = lastVal;

	if(
		(*(int *)(buf) != firstVal) ||
		(*(int *)(buf + size - sizeof(int)) != lastVal))
	{
		cout << "Memory rw TEST error" << endl;
		return -1;
	}

	return 0;
}

void prinHex(char *buf, int size)
{
	for(int i = 0; i < size; i++){
		cout << hex << std::setw(2) << setfill('0') << (0xff & (int)buf[i]) << " ";
	}
	cout << dec;
}



// dataGen
// Data generator
// char *buf -
// unsigned int len - 
// int ID -
void dataGen(char *buf, int ID, unsigned int len){
	char counter = 0;
#ifdef __COUNT_DATA__
	for(int i = 0; i < len; i += 4){
		*(int *)(buf + i) = i/4 + 0x10;
	}  
#else	
	do {		
		int state = len & 0x03;
		*(buf++) = (state == 0 ) ? ID : 
			(state == 1 ) ? (char) (counter) : 
			(state == 2 ) ? 0xA5 : 
			(char)rand();
		
		if(state == 1)
			counter++;
	}
	while(--len);
#endif
}

// Speed calculator
// unsigned int byteNum - number of bytes
// long long int optime - operation time in us
double speedCalc(unsigned int byteNum, long long int optime){
	double speed = (double) byteNum / (double) optime; // byte/us
	speed *= 8 * 1000000/1024; // convert to kbit/s
	return speed;
}
