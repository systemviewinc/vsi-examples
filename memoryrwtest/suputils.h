#ifndef SUPUTILS_H
#define SUPUTILS_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstddef>

using namespace std;

void prinFirstLastHex(char *buf, int size, int num);
void prinFirstLastHex(char *buf, int size, int num);
void printHex32_2col(char *buf, char *buf2, int size);
int mempretest(char *buf, int size);
void prinHex(char *buf, int size);
void dataGen(char *buf, int ID, unsigned int len);
double speedCalc(unsigned int byteNum, long long int optime);
void speeddatahnd(unsigned int transID,
	long long int writetime,
	long long int readtime,
	unsigned int bufSize);
int datavalidcheck(int attemptID, char * readBuf, 
	char * writeBuf, unsigned int bufSize);

#endif
