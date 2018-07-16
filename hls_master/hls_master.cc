#include <vsi_device.h>
#include <unistd.h>


void hls_master (vsi::device &mem)
{
	char buff[10*4];
	for (int i = 0 ; i < 10*4; i++) buff[i] = i;
	mem.pwrite(buff,sizeof(buff),0);
}
