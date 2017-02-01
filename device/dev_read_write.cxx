#include <unistd.h>
#include <string.h>
#include <vsi_device.h>
#include <chrono>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#include "dev_read_write.h"

using namespace std::chrono;

void dev_read_write (vsi::device &dev)
{
	unsigned int val = 0;
	unsigned int rv ;
	printf("%s: Started \n",__FUNCTION__);
	while(1) sleep(1);
	dev.pwrite(&val,sizeof(val),0);
	val = 1;
	while (1) {
		if (val == 0x080) val = 1;
		else val <<= 1;
		dev.pwrite(&val,sizeof(val),0);
		dev.pread (&rv, sizeof(rv), 0);
		printf("%s : wrote 0x%x read 0x%x\n",__FUNCTION__,val,rv);
		sleep(1);
		printf("%s: woken up ..\n",__FUNCTION__);
	}
}

void mem_read_write(vsi::device &mem)
{
	static char buff[4096];
	static char rbuff[4096];
	char c = 'a';
	printf("%s: Started \n",__FUNCTION__);
	while(1) sleep(1);
	while (1) {
		memset(buff,c,sizeof(buff));
		mem.pwrite(&buff,sizeof(buff),0);
		mem.pread (&rbuff,sizeof(rbuff),0);
		if (memcmp(rbuff,buff,sizeof(buff)))
			printf("%s : Error result does not compare\n",__FUNCTION__);
		if (c == 'z') c = 'a';
		else c++;
		sleep(1);
		printf("%s: woken up ..\n",__FUNCTION__);
	}
}


static float initialize_servo(int min_pw, int max_pw, int pc, int max_angle,int init_pw,vsi::device &atm)
{
	// calibrate 
	// start timer in counter mode and check
	unsigned int TCSR0, TCR0 , TLR0;
	unsigned int TCSR1, TCR1 , TLR1, rv;
	
	TLR0 =  0; // clear the reload register
	atm.pwrite(&TLR0,sizeof(TLR0),4); // offset 4
	
	TCSR0 =  1<<8 ; // clear interrupt
	TCSR0 |= 1<<5 ; // load counter from TLR0
	atm.pwrite(&TCSR0,sizeof(TCSR0),0); // offset 0
	
	TCSR0 =  1<<7;	// start the timer
	atm.pwrite(&TCSR0,sizeof(TCSR0),0); // offset 0
	
	high_resolution_clock::time_point t2;
	microseconds diff_time;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	do {
		t2 = high_resolution_clock::now();
		atm.pread (&TCR0,sizeof(TCR0), 8); // read counter : offset 8
		diff_time = duration_cast<microseconds>(t2 - t1);
	} while (diff_time.count() < 500000) ; // for .5 seconds
	
	// TCR0 has timer value for .5 seconds
	printf("%s: timer cycles %d for %lld microseconds\n",__FUNCTION__,TCR0,diff_time.count());
	
	// TLR0 =  0; // clear the reload register
	// atm.pwrite(&TLR0,sizeof(TLR0),4);    // offset 4	
	// atm.pwrite(&TLR0,sizeof(TLR0),0x14); // offset 0x14 : TLR1
	
	// initial reload values
	// Timer 0 will generate the Pule Cycle Frequency
	TLR0 = ((long long)TCR0/diff_time.count())*pc;
	TLR0 = (unsigned int)0xffffffff - TLR0;
	
	// Initialize pulse width to midpoint       
	TLR1 = ((long long)TCR0/diff_time.count())*init_pw;
	TLR1 = (unsigned int)0xffffffff - TLR1;
	
	printf("%s : TLR0 %d , TLR1 %d\n", __FUNCTION__, TLR0, TLR1);
	
	atm.pwrite(&TLR0,sizeof(TLR0),4);    // offset 4	
	atm.pwrite(&TLR1,sizeof(TLR1),0x14); // offset 0x14 : TLR1
	
	TCSR0 =  1<<8 ; // clear interrupt
	TCSR0 |= 1<<5 ; // load counter from TLR0
	TCSR0 |= 1<<9 ; // PWM Mode
	TCSR0 |= 1<<2 ; // Enable Generate
	TCSR0 |= 1<<4 ; // Auto Reload
	atm.pwrite(&TCSR0,sizeof(TCSR0),0);    // offset 0
	atm.pwrite(&TCSR0,sizeof(TCSR0),0x10); // offset 0x10 : TCSR for timer 1
	
	// clear load flags
	TCSR0 &= ~(1<<5);
	atm.pwrite(&TCSR0,sizeof(TCSR0),0);    // offset 0
	atm.pwrite(&TCSR0,sizeof(TCSR0),0x10); // offset 0x10 : TCSR for timer 1
	
	atm.pread (&TCSR0,sizeof(TCSR0),0);    // offset 0
	TCSR0 |= 1 << 10; // enable both
	atm.pwrite(&TCSR0,sizeof(TCSR0),0);    // offset 0

	sleep(1); // wait for servo to sync up
	unsigned int min_tlr = ((long long)TCR0/diff_time.count())*(min_pw);
	unsigned int max_tlr = ((long long)TCR0/diff_time.count())*(max_pw);
	return (float)(max_tlr - min_tlr)/(float) max_angle; // return tlr count per degree
}

template<int min_pw,int max_pw,int pc,int max_deg, int init_pw, int ia> void servo_motor(hls::stream<servo_command> &s_cmd,
								    vsi::device &atm)
{
	// int min_pw = 900;   // uS	minimum Pulse width 0 degree
	// int max_pw = 2100;  // uS	maximum Pulse width 60 degree
	// int pc     = 20000; // uS 	Pulse Cycle Frequency
	// int max_angle = 90;
	int c_angle;
	float c_p_a;
	printf("%s : thread started\n", __FUNCTION__);

	c_p_a   = initialize_servo(min_pw, max_pw, pc, max_deg, init_pw,atm);
	c_angle = ia; // initialized to center

	printf("%s : init complete %f %d\n", __FUNCTION__, c_p_a, c_angle);
	// process the commands as they arrive
	while (1) {
		bool done = false;
		printf("Before read\n");
		servo_command sc = s_cmd.read();
		bool inc = (c_angle < sc.angle);
		unsigned int c_tlr, tlr_diff = sc.incr * c_p_a;		
		atm.pread(&c_tlr,sizeof(c_tlr),0x14); // offset 0x14 : TLR1
		c_tlr = (unsigned int)0xffffffff - c_tlr;
		printf("%s : got command ca %d, sc.angle %d, sc.incr %d, sc.delay %d tlr_diff %d \n",
		       __FUNCTION__, c_angle, sc.angle, sc.incr, sc.delay, tlr_diff );
		// execute the command
		do {
			if (inc) {
				c_tlr += (tlr_diff);
				c_angle += sc.incr;
				done = c_angle >= sc.angle;
			} else {
				c_tlr -= (tlr_diff);
				c_angle -= sc.incr;
				done = c_angle <= sc.angle;
			}
			unsigned int w_c_tlr = (unsigned int) 0xffffffff - c_tlr;
			//printf("%s : c_tlr %d, c_angle %d , done %d\n",__FUNCTION__, c_tlr, c_angle, done);
			atm.pwrite(&w_c_tlr,sizeof(w_c_tlr),0x14);
			usleep(sc.delay);
		} while (!done);
	}
}

void servo_HS55(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<700,2300,10000,180,1500,90>(s_cmd,mot);
}

void servo_HS805BB(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<1100,1700,20000,70,1200,0>(s_cmd,mot);
}

void servo_HS775HB(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<800,1550,20000,90,800,0>(s_cmd,mot);
}

void servo_HS645MG(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<800,2270,20000,160,800,0>(s_cmd,mot);
}



void trajectory_generator(hls::stream<servo_command> &base,
			  hls::stream<servo_command> &shoulder,
			  hls::stream<servo_command> &elbow,
			  hls::stream<servo_command> &wrist)
{
	servo_command base_sc;
	servo_command shoulder_sc;
	servo_command elbow_sc;
	servo_command wrist_sc;
	sleep(5);
	printf("%s : started .. \n",__FUNCTION__);
	while(1) {
		// take base to zero
		base_sc.angle = 0;
		base_sc.incr  = 1;
		base_sc.delay = 60000;
		base.write(base_sc);
		base_sc.angle = 90;
		base.write(base_sc);

		// take base to zero
		shoulder_sc.angle = 30;
		shoulder_sc.incr  = 1;
		shoulder_sc.delay = 200000;
		shoulder.write(shoulder_sc);
		shoulder_sc.angle = 0;
		shoulder.write(shoulder_sc);

		//elbow
		elbow_sc.angle = 40;
		elbow_sc.incr  = 1;
		elbow_sc.delay = 20000;
		elbow.write(elbow_sc);
		elbow_sc.angle = 0;
		elbow.write(elbow_sc);

		//wrist
		wrist_sc.angle = 0;
		wrist_sc.incr  = 1;
		wrist_sc.delay = 20000;
		wrist.write(wrist_sc);
		wrist_sc.angle = 150;
		wrist.write(wrist_sc);
		sleep(1);

		while(!shoulder.empty() ||
		      !base.empty()     ||
		      !wrist.empty()    ||
		      !elbow.empty()); // wait for base to finish
	}
}


void trajectory_passthru (hls::stream<servo_command> &command_in,
			  hls::stream<servo_command> &base,
			  hls::stream<servo_command> &shoulder,
			  hls::stream<servo_command> &elbow,
			  hls::stream<servo_command> &wrist)
{
	servo_command sc_in = command_in.read();
	switch(sc_in.motor_n) {
	case 0:
		base.write(sc_in);
		break;
	case 1:
		shoulder.write(sc_in);
		break;
	case 2:
		elbow.write(sc_in);
		break;
	case 3:
		wrist.write(sc_in);
		break;
	default:
		while(!shoulder.empty() ||
		      !base.empty()     ||
		      !wrist.empty()    ||
		      !elbow.empty()); // wait for base to finish
		break;
	}
}
