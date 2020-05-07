#include <cardano.h>


void processData(input_window_int32  *s1)
{ 
	// write -1 in first 16 locations
	// Just for illustration.  You can write
	// your compute.  The result will be 
	// visible to the master. 
	
	int i;
	int32 *p = (int32 *) s1->head; 
	for (i = 0; i < 16; ++i) { 
		*p++ = -1; 
	} 
} 

void slaveKernel(input_window_int32 * s1)
{
	// get the read lock for s1.
	
	acquire(s1->lockids[0],1);
	printf ("\nSlave got the lock\n"); 
	processData(s1);
	release(s1->lockids[0],0);
	printf ("Slave released the lock\n"); 
	// release the read lock
}
