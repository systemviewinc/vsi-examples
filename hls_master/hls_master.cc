#include <vsi_device.h>
#include <unistd.h>
#include <hls_stream.h>

#define LOOP_COUNT 10

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
	for (int i = 0 ; i < 40; i++) {
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
    int ret = 0;
    bool failed = false;
    for(int i = 0; i < LOOP_COUNT; i++) {
        printf("\n[Controller]----------SW READ COUNTER----------\n");
        out_1.write((int)1);
        failed =  in_1.read() || failed  ? true : false;
        printf("[Controller]---------------------------------------\n");
        printf("\n[Controller]----------HW READ COUNTER----------\n");
        out_2.write((int)2);
        failed =  in_2.read() || failed ? true : false;
        printf("[Controller]---------------------------------------\n");
        printf("\n[Controller]----------HW READ/WRITE ADDER----------\n");
        out_3.write((int)3);
        failed =  in_3.read()  || failed ? true : false;
        printf("[Controller]---------------------------------------\n");
    }
    if (failed)
        printf("[Controller] Failed!\n");
    else
        printf("[Controller] Passed!\n");
    sleep(5);
#ifndef __VSI_HLS_SYN__
    exit(0);
#endif

}


//different print to see
void hls_reader_controlled(hls::stream<int> &input, vsi::device mem, hls::stream<int> &output)
{
    int num = input.read();
	int buff[10*4];
    bool failed = false;
	mem.pread(buff,sizeof(buff),0);
	printf("[reader_%i]-----READ BUFFER-----\n", num);

	for (int i = 0 ; i < 10*4; i++) {
		printf("[reader_%i]%d : %d \n", num, i, buff[i]);
        if(buff[i] != i) {
            failed = true;
            printf("[reader_%i]\tFail! \n", num);
        }
    }
	printf("[reader_%i]---------------------\n", num);

    if(failed) {
        output.write((int)-1);
    }
    else {
        output.write((int)0);
    }
}

void hls_reader_writer_controlled(hls::stream<int> &input, vsi::device mem, hls::stream<int> &output)
{
    int num = input.read();
	int write_buff[40];
	int read_buff[40];
    bool failed = false;

	printf("[read/write_%i]-----WRITE BUFFER-----\n", num);
	for (int i = 0 ; i < 40; i++) {
		write_buff[i] = 5*count++;
		printf("[read/write_%i]%d : %d \n", num, i, write_buff[i]);
	}
	mem.pwrite(&write_buff,sizeof(write_buff),0);
	printf("[read/write_%i]---------------------\n", num);
    //wait some time for hls block to be able to process
    sleep(1);
	mem.pread(&read_buff,sizeof(read_buff),sizeof(write_buff));
	printf("[read/write_%i]-----READ BUFFER-----\n", num);
	for (int i = 0 ; i < 40; i++){
		printf("[read/write_%i]%d : %d \n", num, i, read_buff[i]);
        if(read_buff[i] != write_buff[i]+10) {
            failed = true;
            printf("[read/write_%i]\tFail! \n", num);
        }
    }
	printf("[read/write_%i]---------------------\n", num);

    if(failed) {
        output.write((int)-1);
    }
    else {
        output.write((int)0);
    }
}

#include <hls_master.h>

// shared memory as an array
void shmem_array (hls::stream<axis_dl> &start,
		  int sh_mem[256][256],
		  hls::stream<axis_dl> &done) {
	axis_dl ds = start.read();

	for (int i = 0 ; i < 256; i ++) {
		for (int j = 0 ; j < 256; j++) {
			sh_mem[i][j] = i*j;
		}
	}
	axis_dl dd;
	dd.data = 1024;
	dd.last = 1;
	done.write(dd);
}

// shared memory as an array : add
void shmem_array_add (hls::stream<axis_dl> &start,
		  int sh_mem[256][256],
		  hls::stream<axis_dl> &done) {
	axis_dl ds = start.read();

	for (int i = 0 ; i < 256; i ++) {
		for (int j = 0 ; j < 256; j++) {
			sh_mem[i][j] = i+j;
		}
	}
	axis_dl dd;
	dd.data = 1024;
	dd.last = 1;
	done.write(dd);
}

#ifndef __VSI_HLS_SYN__
#include <unistd.h>
// software portion of the test
void shmem_array_sw (hls::stream<axis_dl> &start,
		     vsi::device &sh_mem_dev,
		     hls::stream<axis_dl> &done) {
	static int sh_mem[256][256];
	axis_dl ds ;
	ds.data = 1024;
	ds.last = 1;
	start.write(ds);
	// wait for it to end
	axis_dl dd = done.read();
	printf("%s: Got done \n",__FUNCTION__);
	sh_mem_dev.pread(sh_mem,sizeof(sh_mem),0);
	for (int i = 0 ; i < 256; i ++) {
		for (int j = 0 ; j < 256; j++) {
			if (sh_mem[j][i] != i*j) {
				printf("Mismatch @ [%d][%d] expected %d, got %d\n",i,j,i*j,sh_mem[i][j]);
			}
		}
	}
	printf("%s: All Ok \n",__FUNCTION__);
	while(1) sleep(1);
}

void shmem_array_sw_add (hls::stream<axis_dl> &begin,
			 hls::stream<axis_dl> &start,
			 vsi::device &sh_mem_dev,
			 hls::stream<axis_dl> &done) {
	static int sh_mem[256][256];
	axis_dl ds, bs ;
	// wait for the begin
	bs = begin.read();

	ds.data = 1024;
	ds.last = 1;
	start.write(ds);
	// wait for it to end
	axis_dl dd = done.read();
	printf("%s: Got done \n",__FUNCTION__);
	sh_mem_dev.pread(sh_mem,sizeof(sh_mem),0);
	for (int i = 0 ; i < 256; i ++) {
		for (int j = 0 ; j < 256; j++) {
			if (sh_mem[j][i] != i+j) {
				printf("Mismatch @ [%d][%d] expected %d, got %d\n",i,j,i+j,sh_mem[i][j]);
			}
		}
	}
	printf("%s: All Ok\n",__FUNCTION__);
	while(1) sleep(1);
}

void broadcast (hls::stream<axis_dl> &in,
		hls::stream<axis_dl> &out_1,
		hls::stream<axis_dl> &out_2) {
	do {
		axis_dl ind = in.read();
		out_1.write(ind);
		out_2.write(ind);
	} while (!in.empty());
}

void five_memories(vsi::device &mem1, vsi::device &mem2, vsi::device &mem3,vsi::device &mem4,vsi::device &mem5 ,vsi::device &dev, vsi::device &dev1, vsi::device &dev2)
{
	static int buff[1024];

	mem1.pread(buff,sizeof(buff),0);
	mem2.pwrite(buff,sizeof(buff),0);
	mem1.pwrite(buff,sizeof(buff),0);
	mem3.pread(buff,sizeof(buff),0);
	mem2.pwrite(buff,sizeof(buff),0);
	mem3.pwrite(buff,sizeof(buff),0);
	mem4.pread(buff,sizeof(buff),0);
	mem2.pwrite(buff,sizeof(buff),0);
	mem4.pwrite(buff,sizeof(buff),0);
	mem5.pread(buff,sizeof(buff),0);
	mem2.pwrite(buff,sizeof(buff),0);
	mem5.pwrite(buff,sizeof(buff),0);
}
#endif
