#include "adc_spi.h"
#ifndef __VSI_HLS_SYN__
void send_control (unsigned int *control)
{
	static int state = 0;
	if (state == 0) {
		*control=1;
		state = 1;
		printf("Sent state 1\n");
	} else if (state == 1) {
		*control=2;
		printf("Sent state 2\n");
		state = 2;
	} else {
		printf("going to sleep\n");
		while (1) sleep(10);
	}
	
}
#endif
static void wait_dr_low(vsi::device &adc_ctrl)
{
	uint32_t cr = 0, cr_p = 0;
	// wait for /DR
	cr = cr_p = 0;
	while(1) {
		adc_ctrl.pread(&cr,sizeof(cr),8);
		if ((cr_p & 1) && !(cr & 1)) break ; // falling edge
		cr_p = cr;
	}
}

static void enable_slave(vsi::device &adc, unsigned int slave)
{
	uint32_t wv;
	adc.pread(&wv,sizeof(wv),0x70);
	wv &= ~slave;
	adc.pwrite(&wv,sizeof(wv),0x70);
}

static void disable_slave(vsi::device &adc, unsigned int slave)
{
	uint32_t wv;
	adc.pread(&wv,sizeof(wv),0x70);
	wv |= slave;
	adc.pwrite(&wv,sizeof(wv),0x70);
}

static void enable_master(vsi::device &adc)
{
	uint32_t wv;
	adc.pread(&wv,sizeof(wv),0x60);
	wv &= ~(1 << 8); // enable master
	adc.pwrite(&wv,sizeof(wv),0x60);
}
static void disable_master(vsi::device &adc)
{
	uint32_t wv;
	adc.pread(&wv,sizeof(wv),0x60);
	wv |= (1 << 8); // disable master
	adc.pwrite(&wv,sizeof(wv),0x60);
}

static void send_command(unsigned int cmds[], int ncommands, vsi::device &adc)
{
	uint32_t wv;
	
	for (int i = 0 ; i < ncommands ; i++) {
		adc.pwrite(&cmds[i],sizeof(cmds[i]),0x68); // write command into FIFO
	}
	// enable the slave
	enable_slave(adc,1);
	enable_master(adc);
	do {
		adc.pread(&wv,sizeof(wv),0x64);
	} while (wv & 1); // wait receive ~empty
	adc.pread(&wv,sizeof(wv),0x6c); // read value
	disable_master(adc);
	disable_slave(adc,1);
}

static void read_registers(unsigned int reg, int nregs, unsigned int reg_vals[], vsi::device &adc )
{
	uint32_t wv;

	// put commands into fifo
	wv = reg | (1 << 4);
	adc.pwrite(&wv,sizeof(wv),0x68); // RREG & starting reg
	wv = nregs-1; 
	adc.pwrite(&wv,sizeof(wv),0x68); // number of regs

	enable_slave(adc,1);
	usleep(5); // wait a little
	for (int i = 0 ; i < nregs; i++) {
		wv = 0; 		
		adc.pwrite(&wv,sizeof(wv),0x68); // nop
	}
	enable_master(adc);
	for (int i = 0 ; i < nregs; i++) {
		do {
			adc.pread(&wv,sizeof(wv),0x64);
		} while (wv & 1); // wait receive ~empty
		adc.pread(&wv,sizeof(wv),0x6c); // read value
		reg_vals[i] = wv;
	}
	disable_master(adc);
	disable_slave(adc,1);
}

void get_adcsample_hw(vsi::device adc, vsi::device adc_ctrl, unsigned int *control)
{
	static int state  = 0;
	uint32_t wv = 0;
	uint32_t cr = 0, cr_p = 0;
	unsigned int cmd_arr[16];
	
	int internal_control = *control;
	
	printf("internal control = %d\n",internal_control);
	switch (state) {
	case 0:
		if (internal_control == 1) // intialize;
			state = 1;
		break;
	case 1:
		printf("Case 1\n");
		wv = 0xf;
		adc.pwrite(&wv,sizeof(wv),0x70); // disable slave
		wv = 0x1f6; // inhibit master : manual slave select : reset fifos : cpha = 1 : master mode : enable core
		adc.pwrite(&wv,sizeof(wv),0x60);
		adc.pread(&wv,sizeof(wv),0x60);
		state = 2;
		wait_dr_low(adc_ctrl);
		// send RESET Command
		cmd_arr[0] = 0xFE;
		send_command(cmd_arr,1,adc);
		break;

	case 2:		
		if (internal_control == 2) {// start reading
			read_registers(0,8,cmd_arr,adc);
			for(int i = 0 ; i < 8; i++) {
				printf("register %d = 0x%x\n",i,cmd_arr[i]);
			}
			state = 2;
		}
		break;

	case 3:
		printf("Case 3\n");
		printf("Falling edge cmd\n");
		wait_dr_low(adc_ctrl);
		// send RDATAC command
		cmd_arr[0] = 0x3;
		send_command(cmd_arr,1, adc);
		
		printf("Reading samples\n");
		for (int i = 1 ; i <= SAMPLES; i++) {
			unsigned int sample = 0;
			fft_data_s fft_d;
			wait_dr_low(adc_ctrl);
			// read 24 bits
			wv = 0;
			for (int j = 0 ; j < 3; j++)
				adc.pwrite(&wv,sizeof(wv),0x68); // dummy write
			adc.pread(&wv,sizeof(wv),0x60);
			wv &= ~(1 << 8); // enable master
			adc.pwrite(&wv,sizeof(wv),0x60);
			for (int j = 0 ; j < 3; j++) {
				do {
					adc.pread(&wv,sizeof(wv),0x64);
				} while (wv & 1); // wait receive ~empty
				adc.pread(&wv,sizeof(wv),0x6c); // read value
				sample |= (wv << (8*j));
			}
			// disable master
			adc.pread(&wv,sizeof(wv),0x60);
			wv |= (1 << 8); // disable master
			adc.pwrite(&wv,sizeof(wv),0x60);
		}
		wait_dr_low(adc_ctrl);
		// send SDATAC command
		cmd_arr[0] = 0xf;
		send_command(cmd_arr,1, adc);
		
		if (internal_control == 3) state = 4; // pause
		printf("Done\n");
		break;
	case 4:
		if (internal_control == 4) state = 3; // restart
		break;
	}
	*control = state;
}
