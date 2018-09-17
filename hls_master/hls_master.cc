#include <vsi_device.h>
#include <unistd.h>
#include <hls_stream.h>


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

void hls_master_random (vsi::device mem)
{
	int buff[10*4];
	for (int i = 0 ; i < 10*4; i++) buff[i] = rand_int();
	mem.pwrite(buff,sizeof(buff),0);
}

void hls_master(vsi::device mem)
{
	int buff[10*4];
	for (int i = 0 ; i < 10*4; i++) buff[i] = i;
	mem.pwrite(buff,sizeof(buff),0);
}

void hls_master_adder(vsi::device mem)
{
	int buff[10*4];
	mem.pread(buff,sizeof(buff),0);
	for (int i = 0 ; i < 10*4; i++) {
		buff[i] += 10;
	}
	mem.pwrite(buff,sizeof(buff),sizeof(buff));
}


void hls_reader(vsi::device mem)
{
	int buff[10*4];
	mem.pread(buff,sizeof(buff),0);
	printf("[reader]-----READ BUFFER-----\n");
	for (int i = 0 ; i < 10*4; i++)
		printf("[reader]%d : %d \n", i, buff[i]);
	printf("[reader]---------------------\n");

}

void hls_reader_writer(vsi::device mem)
{
	int write_buff[40];
	int read_buff[40];
	printf("[read/write]-----WRITE BUFFER-----\n");
	for (int i = 0 ; i < 10*4; i++) {
		write_buff[i] = count++;
		printf("[read/write]%d : %d \n", i, write_buff[i]);
	}
	mem.pwrite(&write_buff,sizeof(write_buff),0);
	printf("[read/write]---------------------\n");

	mem.pread(&read_buff,sizeof(read_buff),sizeof(write_buff));
	printf("[read/write]-----READ BUFFER-----\n");
	for (int i = 0 ; i < 40; i++)
		printf("[read/write]%d : %d \n", i, read_buff[i]);
	printf("[read/write]---------------------\n");

}

//different print to see
void system_controller(hls::stream<int> &out_1, hls::stream<int> &out_2, hls::stream<int> &out_3,
                        hls::stream<int> &in_1, hls::stream<int> &in_2, hls::stream<int> &in_3)
{
    printf("\n\n\n\n[Controller]----------SW READ COUNTER----------\n");
    out_1.write((int)1);
    in_1.read();
    printf("[Controller]---------------------------------------\n");

    getchar();
    printf("\n\n\n\n[Controller]----------HW READ RANDOM----------\n");
    out_2.write((int)2);
    in_2.read();
    printf("[Controller]---------------------------------------\n");
    getchar();
    printf("\n\n\n\n[Controller]----------HW READ/WRITE ADDER----------\n");
    out_3.write((int)3);
    in_3.read();
    printf("[Controller]---------------------------------------\n");

    getchar();
}


//different print to see
void hls_reader_controlled(hls::stream<int> &input, vsi::device mem, hls::stream<int> &output)
{
    int num = input.read();
	int buff[10*4];
	mem.pread(buff,sizeof(buff),0);
	printf("[reader_%i]-----READ BUFFER-----\n", num);
	for (int i = 0 ; i < 10*4; i++)
		printf("[reader_%i]%d : %d \n", num, i, buff[i]);
	printf("[reader_%i]---------------------\n", num);
    output.write((int)1);


}

void hls_reader_writer_controlled(hls::stream<int> &input, vsi::device mem, hls::stream<int> &output)
{
    int num = input.read();
	int write_buff[40];
	int read_buff[40];
	printf("[read/write_%i]-----WRITE BUFFER-----\n", num);
	for (int i = 0 ; i < 10*4; i++) {
		write_buff[i] = 5*count++;
		printf("[read/write_%i]%d : %d \n", num, i, write_buff[i]);
	}
	mem.pwrite(&write_buff,sizeof(write_buff),0);
	printf("[read/write_%i]---------------------\n", num);
    //sleep(1);
	mem.pread(&read_buff,sizeof(read_buff),sizeof(write_buff));
	printf("[read/write_%i]-----READ BUFFER-----\n", num);
	for (int i = 0 ; i < 40; i++)
		printf("[read/write_%i]%d : %d \n", num, i, read_buff[i]);
	printf("[read/write_%i]---------------------\n", num);
    output.write((int)1);
}
