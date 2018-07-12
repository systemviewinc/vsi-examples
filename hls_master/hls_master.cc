#include <vsi_device.h>
#include <unistd.h>


void hls_master (vsi::device &mem)
{
	int buff[10];
	for (int i = 0 ; i < 10; i++) buff[i] = i;
	mem.pwrite(buff,sizeof(buff),0);
}
