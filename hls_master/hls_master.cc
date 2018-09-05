#include <vsi_device.h>
#include <unistd.h>

static int rnd_seed = 402143098;
static int count = 0;



void set_rnd_seed (int new_seed)
{
    rnd_seed = new_seed;
}

int rand_int (void)
{
    int k1;
    int ix = rnd_seed;

    k1 = ix / 127773;
    ix = 16807 * (ix - k1 * 127773) - k1 * 2836;
    if (ix < 0)
        ix += 2147483647;
    rnd_seed = ix;
    return rnd_seed;
}

void hls_master (vsi::device mem)
{
	int buff[10*4];
	for (int i = 0 ; i < 10*4; i++) buff[i] = rand_int();
	mem.pwrite(buff,sizeof(buff),0);
}

void hls_master_random (vsi::device mem)
{
	int buff[10*4];
	for (int i = 0 ; i < 10*4; i++) buff[i] = i;
	mem.pwrite(buff,sizeof(buff),0);
}

void hls_master_adder (vsi::device mem)
{
	int buff[10*4];
	mem.pread(buff,sizeof(buff),0);
	for (int i = 0 ; i < 10*4; i++) {
		buff[i] += 10;
	}
	mem.pwrite(buff,sizeof(buff),40*sizeof(int));
}


void hls_reader (vsi::device mem)
{
	int buff[10*4];
	mem.pread(buff,sizeof(buff),0);
	printf("[reader]-----READ BUFFER-----\n");
	for (int i = 0 ; i < 10*4; i++)
		printf("[reader]%d : %d \n", i, buff[i]);
	printf("[reader]---------------------\n");

}

void hls_reader_writer (vsi::device mem)
{
	int buff[10*4];
	int read_buff[10*8];
	printf("[read/write]-----WRITE BUFFER-----\n");
	for (int i = 0 ; i < 10*4; i++) {
		buff[i] = count++;
		printf("[read/write]%d : %d \n", i, buff[i]);
	}
	mem.pwrite(&buff,sizeof(buff),0);
	printf("[read/write]---------------------\n");

	mem.pread(&read_buff,sizeof(read_buff),0);
	printf("[read/write]-----READ BUFFER-----\n");
	for (int i = 0 ; i < 10*8; i++)
		printf("[read/write]%d : %d \n", i, read_buff[i]);
	printf("[read/write]---------------------\n");

}
