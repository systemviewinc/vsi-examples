#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "mem_rwtest.h"
#include "suputils.h"

#include <syslog.h>

using namespace std;
using namespace std::chrono;

#ifndef __VSI_HLS_SYN__

#ifdef INTERNAL_MEM
char shareMemTet[CHUNK_SIZE * MAX_CHUNK];
#endif

// controller__
// Benchmark controller
// mutex *thrm
// struct benchTestData *thrData
void controller__(mutex *thrm, struct benchTestData *thrData)
{
	srand(1); // initialize random seed

	while(thrData->bufSize <= MAX_CHUNK * CHUNK_SIZE)    // stay in a loop until r/w bufSize not reach maximal size
	{
		int attempt  = TEST_NUM;
		thrData->readtime = -1; // reset time mesures
		thrData->writetime = -1;
		thrData->writeID = 0; // reset r/w ID's
		thrData->readID = 0;

		while(attempt)
		{
			thrm->lock();

			if(thrData->writeID == thrData->readID) // enable write process just if r/w ID's is equal
			{
				thrData->writeProc = true;
			}

			if((thrData->readtime > -1) && (thrData->writetime > -1))   // start data analyze when r/w time present
			{
				speeddatahnd(thrData->writeID, thrData->writetime, thrData->readtime, thrData->bufSize);
				thrData->writetime = -1; // time mesure reset
				thrData->readtime = -1;
				datavalidcheck(attempt, thrData->readBuf, thrData->writeBuf, thrData->bufSize); // Data validation
				free(thrData->writeBuf);
				free(thrData->readBuf);
				attempt--;
			}

			thrm->unlock();
		}
		thrm->lock();
		thrData->writeProc = false; // swich off any write process
		thrData->readProc = false; // swich off any write process
		thrData->bufSize *= 2;
		thrm->unlock();
	}
	thrData->inprocess = false;    // out of process
}

void read__(mutex *thrm, struct benchTestData *thrData, vsi::device<int> *mem)
{
	while(1){
		thrm->lock();
		if(thrData->readProc){
#ifdef WAIT_PRESS_KEY
			cout << "Ready to read? " << endl;
			getchar();
#endif

			thrData->readBuf = (char*)malloc(thrData->bufSize * sizeof(char));
			memset(thrData->readBuf, 0xDC, thrData->bufSize);
			high_resolution_clock::time_point startime = high_resolution_clock::now(); // set start time point

#ifdef SUPPORT_MSG
			cout << "Try to read: " << thrData->bufSize << " bytes " << endl;
#endif

#ifdef INTERNAL_MEM
			memcpy(thrData->readBuf, shareMemTet, thrData->bufSize);
#else
			mem->pread(thrData->readBuf, thrData->bufSize, 0); // read from offset
#endif

			high_resolution_clock::time_point endtime = high_resolution_clock::now(); // set end time point
			thrData->readtime = duration_cast<microseconds>( endtime - startime ).count();
			thrData->readProc = false; // disabel read process
			thrData->readID++;

#ifdef SUPPORT_MSG
			cout << "Read done" << endl;
#endif
		}
		thrm->unlock();
	}
}

void write__(mutex *thrm, struct benchTestData *thrData, vsi::device<int> *mem)
{
	while(1) {
		thrm->lock();
		if(thrData->writeProc)
		{

#ifdef WAIT_PRESS_KEY
			cout << "Ready to write? " << endl;
			getchar();
#endif
			thrData->writeBuf = (char*)malloc(thrData->bufSize * sizeof(char));
			dataGen(thrData->writeBuf, thrData->writeID, thrData->bufSize);
			high_resolution_clock::time_point startime = high_resolution_clock::now();
#ifdef SUPPORT_MSG
			cout << "Try to write: " << thrData->bufSize << " bytes " << endl;
#endif

#ifdef INTERNAL_MEM
			memcpy(shareMemTet, thrData->writeBuf, thrData->bufSize);
#else
			mem->pwrite(thrData->writeBuf, thrData->bufSize, 0); // write to mem by offset
#endif
			high_resolution_clock::time_point endtime = high_resolution_clock::now();
			thrData->writetime = duration_cast<microseconds>(endtime - startime).count();
			thrData->readProc = true; // enable read process
			thrData->writeProc = false; // disabel write process
			thrData->writeID++;

#ifdef SUPPORT_MSG
			cout << "Write done" << endl;
#endif
		}
		thrm->unlock();
	}
}

// Memory benchmark
void memoryBenchmark(vsi::device<int> &mem)
{

	struct benchTestData thrData;// Initialization structure ....
	mutex *threadMutex = new mutex();
	cout << __DATE__ << __TIME__ << endl;
	thread control_th(controller__, threadMutex, &thrData); // Control thread declaration
	thread write_th(write__, threadMutex, &thrData, &mem);  // Memory write thread declaration
	thread read_th(read__, threadMutex, &thrData, &mem);     // Memory read thread declaration
	write_th.join();
	read_th.join();
	control_th.join();
//cout << "The END" << endl;getchar();
	while(thrData.inprocess)
	{ };
}

#endif
