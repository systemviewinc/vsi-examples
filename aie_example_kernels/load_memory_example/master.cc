#include <adf.h>
#define SZ (6*1024)
int32 chess_storage(%chess_alignof(v4int32)) lut0[SZ]; 


// Even though we allocate 24k buffer for data per core,
// The example is hard coded for 16 samples, to 
// keep the simulation time minimal and yet 
// showing the communication and synchronization. 

void masterKernel(input_stream_int32  *s1,
                  output_window_int32 *o0,
                  output_window_int32 *o1,
                  output_window_int32 *o2, 
                  output_stream_int32 *os1) {
	
	v4int32 *p = (v4int32 *) lut0; 
	int i; 
	acquire(o0->lockids[0],0);
	o0->ptr=o0->head;
	for (i=0;i< 16 ;i++) {
		//     v4int32 data = readincr_v4(s1);
		int32 data = readincr(s1);
		window_writeincr(o0, data);
	}
	release(o0->lockids[0],1);
	o0->ptr=o0->head;
	o1->ptr=o1->head;
	acquire(o1->lockids[0],0);
	for (i=0;i< 16 ;i++) {
		//v4int32 data = readincr_v4(s1);
		int32 data = readincr(s1);
		window_writeincr(o1, data);
	}
	o1->ptr=o1->head;
	release(o1->lockids[0],1);
	
	acquire(o2->lockids[0],0);
	o2->ptr=o2->head;
	for (i=0;i< 16 ;i++) {
		// v4int32 data = readincr_v4(s1);
		int32 data = readincr(s1);
		window_writeincr(o2, data);
	}
	o2->ptr=o2->head;
	release(o2->lockids[0],1);
	
	//  p = (v4int32 *) lut0;
	int32 *p0 = (int32 *) lut0;
	for (i=0;i< 16 ;i++) {
		// v4int32 data = readincr_v4(s1);
		int32 data = readincr(s1);
		*p0++ = data; 
	}
	
	// wait for the slaves to finish
	printf("\nWaiting for slaves to finish \n");
	acquire(o0->lockids[0],0);
	acquire(o1->lockids[0],0);
	acquire(o2->lockids[0],0);
	printf("\nSlaves are done \n");
	release(o0->lockids[0],0);
	release(o1->lockids[0],0);
	release(o2->lockids[0],0);
	
	int32 *pi = (int32 *) o0->head;
	
	for (i=0;i< 16 ;i++) {
		//    v4int32 data = *p++;
		// writeincr_v4(os1, data); 
		int32 data = *pi++;
		writeincr(os1, data); 
	}
	
	//  p = (v4int32 *) o1->head;
	pi = (int32 *) o1->head;
	for (i=0;i< 16 ;i++) {
		//    v4int32 data = *p++;
		// writeincr_v4(os1, data); 
		int32 data = *pi++;
		writeincr(os1, data); 
	}
	// p = (v4int32 *) o2->ptr; 
	pi = (int32 *) o2->head;
	for (i=0;i< 16 ;i++) {
		//    v4int32 data = *p++;
		//writeincr_v4(os1, data); 
		int32 data = *pi++;
		writeincr(os1, data); 
	}
	//  p = (v4int32 *) lut0; 
	pi = (int32 *) lut0;
	for (i=0;i< 16;i++) {
		// v4int32 data = *p++;
		// writeincr_v4(os1, data); 
		int32 data = *pi++;
		writeincr(os1, data); 
	}
}
