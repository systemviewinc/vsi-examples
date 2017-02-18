#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <vsi_device.h>
#include <chrono>
#include <map>
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


static float initialize_servo(int min_pw,  int max_pw,   int pc,       int max_angle,
			      int init_pw, int &max_tlr, int &min_tlr, vsi::device &atm)
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
		
	// initial reload values
	// Timer 0 will generate the Pulse Cycle Frequency
	TLR0 = ((long long)TCR0/diff_time.count())*pc;
	TLR0 = (unsigned int)0xffffffff - TLR0;
	
	// Initialize pulse width to specified location       
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
	min_tlr = ((long long)TCR0/diff_time.count())*(min_pw);
	max_tlr = ((long long)TCR0/diff_time.count())*(max_pw);
	return (float)(max_tlr - min_tlr)/(float) max_angle; // return tlr count per degree
}

template<int min_pw,int max_pw,int pc,int max_deg, int init_pw, int ia, bool rev,bool m_debug>
void servo_motor(hls::stream<servo_command> &s_cmd, vsi::device &atm)
{
	int c_angle, m_c_angle;
	float c_p_a;
	bool replay_mode = false;
	int max_tlr, min_tlr;
	
	c_p_a   = initialize_servo(min_pw, max_pw, pc, max_deg, init_pw, max_tlr, min_tlr, atm);
	c_angle = ia; // initialized angle

	if (m_debug)
		printf("%s : init complete %f %d %d %d\n", __FUNCTION__, c_p_a, c_angle, min_tlr, max_tlr);
	// process the commands as they arrive
	while (1) {
		bool done = false;
		servo_command sc ;

		sc = s_cmd.read(); // check the stream
		if (sc.mode == RELATIVE) {
			if (sc.angle == 0) continue;
			if (rev)
				sc.angle = (-sc.angle) + c_angle; // relative
			else
				sc.angle = (sc.angle) + c_angle; // relative
		} else if (sc.mode == MEM_POS) { // memorize current location
			m_c_angle = c_angle;
			continue;
		} else if (sc.mode == POS_MEM) { // reposition to memorized angle
			sc.angle = m_c_angle;
			sc.mode  = NORMAL;
			sc.incr  = 1;
			sc.delay = 10000;
		}
		unsigned int c_tlr, tlr_diff = sc.incr * c_p_a;		

		// offset 0x14 : TLR1
		atm.pread(&c_tlr,sizeof(c_tlr),0x14); 
		c_tlr = (unsigned int)0xffffffff - c_tlr;
		
		sc.angle = sc.angle > max_deg ? max_deg : sc.angle;
		sc.angle = sc.angle < 0 ? 0 : sc.angle;
		bool inc = (c_angle < sc.angle);
		if (m_debug)
		 	printf("%s : got command ca %d, sc.angle %d, sc.incr %d, sc.delay %d dtlr %d inc %d\n",
		 	       __FUNCTION__, c_angle, sc.angle, sc.incr, sc.delay, tlr_diff , inc);
		if (inc)
			done = (c_angle >= sc.angle) || (c_tlr > max_tlr) || c_angle >= max_deg;
		else
			done = (c_angle <= sc.angle) || (c_tlr < min_tlr) || c_angle <= 0;
		// execute the command
		while (!done) {
			if (inc) {
				c_tlr += (tlr_diff);
				c_angle += sc.incr;
				c_angle  = c_angle > max_deg ? max_deg : c_angle;
				done = (c_angle >= sc.angle) || (c_tlr > max_tlr) || c_angle >= max_deg;
			} else {
				c_tlr -= (tlr_diff);
				c_angle -= sc.incr;
				c_angle  = c_angle < 0 ? 0 : c_angle;
				done = (c_angle <= sc.angle) || (c_tlr < min_tlr) || c_angle <= 0;
			}
			unsigned int w_c_tlr = (unsigned int) 0xffffffff - c_tlr;
			atm.pwrite(&w_c_tlr,sizeof(w_c_tlr),0x14);
			usleep(sc.delay);
		} ;
		if (m_debug) printf("%s : c_tlr %d, c_angle %d\n",__FUNCTION__, c_tlr, c_angle);
	}
}

void servo_BASE(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<500,2500,20000,180,1500,90,false,false>(s_cmd,mot);
}

void servo_SHOULDER(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<500,2500,20000,180,1500,90,true,false>(s_cmd,mot);
}

void servo_ELBOW(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<500,2500,20000,110,1500,110,true,false>(s_cmd,mot);
}

void servo_WRIST(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<800,2270,20000,160,800,0,false,false>(s_cmd,mot);
}

static void spi_send_recv_bytes(char *s_bytes, char *r_bytes, int num, vsi::device &spi)
{
	// set ss=0
	int wv = 0;
	spi.pwrite(&wv,sizeof(wv),0x70);
	for (int i = 0 ; i < num; i++) {
		unsigned int sd = s_bytes[i];
		spi.pwrite(&sd,sizeof(sd),0x68);
		do {
			sd = 0;
			spi.pread(&sd,sizeof(sd),0x64);
		} while (!(sd & 0x2));
		spi.pread (&sd,sizeof(sd),0x6c); // read back
		r_bytes[i] = sd;
	}
	// set ss=1
	wv = 1;
	spi.pwrite(&wv,sizeof(wv),0x70);
}

void SPI_JoyStick(vsi::device &spi, hls::stream<js_data> &jsd)
{	
	unsigned int wv = 0x000a;
	unsigned int led = 0x1;
	js_data js, jsp;
	
	printf("%s : started ..\n",__FUNCTION__);
	spi.pwrite(&wv,sizeof(wv),0x40); 	// software reset
	// Control Register
	// LOOP		:0
	// Enable       :1
	// Master	:1
	// Manual SS    :1
	// Rest zeros
	wv = 0x86 | (0x3 << 5);  // reset fifos
	spi.pwrite(&wv,sizeof(wv),0x60);	// write control register	
	memset(&js ,sizeof(js),0);
	memset(&jsp,sizeof(jsp),0);
	
	printf("%s : config complete ..\n",__FUNCTION__);	
	while (1) {
		unsigned char sb[5];
		sb[0] =  0x80 | led;
		led = (led == 1 ? 2 : 1);
		spi_send_recv_bytes((char *)sb,(char *)&js, 5, spi); // send receive 5 bytes
		//printf(" js.X %d js.Y %d jsp.X %d jsp.Y %d\n",js.X, js.Y, jsp.X, jsp.Y);
		// if only button pressed then send only changes
		if (js.Btn_Led & 0x6) {
			if ( js.Btn_Led != jsp.Btn_Led) {			
				//printf("Btn_Led 0x%x 0x%x \n",js.Btn_Led,jsp.Btn_Led);
				jsd.write(js);
			}
		} else jsd.write(js);
		jsp = js;		
		usleep(30000);
	}
}

// joystick entry to angle increment table
static struct a_table {
	short min, max;
	float i_angle;
} angle_table_x[25],  angle_table_y[100];

#define ANGLE_INC_MAX 8

// generate the angle increment table
static void gen_aic(short j_min, short j_max, int a_size, struct a_table *angle_table) {
	int n_entries = a_size;
	float c_angle = ANGLE_INC_MAX;
	short incr    = (j_max - j_min)/n_entries;
	short c_min   = j_min;
	short c_max   = j_min + incr;
	for (int i = 0 ; i < n_entries; i++) {
		angle_table[i].min = c_min;
		angle_table[i].max = c_max;
		angle_table[i].i_angle = c_angle;
		c_min = c_max;
		c_max += incr;
		c_angle -= (2*(float)ANGLE_INC_MAX)/n_entries;
		// printf(" [%d] : min 0x%x max 0x%x angle %f\n",i,
		//        angle_table[i].min,
		//        angle_table[i].max,
		//        angle_table[i].i_angle);
	}
	
}

// get the angle increment from joystick value
static float get_ainc(short val, int a_size, int sens, struct a_table *angle_table)
{
	for (int i = 0 ; i < a_size; i++) {		
		if (val >= angle_table[i].min && val <= angle_table[i].max) {
			// de-sensitize the center of the joy stick
			if (i >= (a_size/2) - sens &&
			    i <= (a_size/2) + sens) return 0.0;
			return angle_table[i].i_angle;
		}
	}
	// out of range
	return 0.0;
}

std::map<int,servo_command[3]> memory;

void trajectory_generator(hls::stream<js_data>       &jsd,
			  hls::stream<servo_command> &base,
			  hls::stream<servo_command> &shoulder,
			  hls::stream<servo_command> &elbow,
			  hls::stream<servo_command> &wrist)
{
	servo_command base_sc;
	servo_command shoulder_sc;
	servo_command elbow_sc;
	servo_command wrist_sc;
	js_data js;
	int mem_idx = 0;
	bool memorize = false;
	bool replay   = false;
	bool replay_reverse = false;
	sleep(1);
	printf("%s : started .. \n",__FUNCTION__);

	// generate angle increment table
	gen_aic(190,900, ARRAY_SIZE(angle_table_x), angle_table_x);
	gen_aic(160,900, ARRAY_SIZE(angle_table_y), angle_table_y);
	while(1) {
		
		// base controlled by joy stick
		while (!jsd.empty()) {			
			js = jsd.read();
			// memorize mode
			if (js.Btn_Led & 0x4) {
				if (memorize) {
					memorize = false;
					base_sc.mode = POS_MEM;
					base_sc.angle = 0;
					shoulder.write(base_sc);
					elbow.write(base_sc);
					base.write(base_sc);
					printf("%s :memorize off\n",__FUNCTION__);
				} else {
					// if memorize being turned on then clear
					if (!memorize) {
						memory.clear();
						mem_idx = 0;
						base_sc.mode = MEM_POS;
						base_sc.angle = 0;
						shoulder.write(base_sc);
						elbow.write(base_sc);
						base.write(base_sc);						
						base_sc.mode = POS_MEM; // reposition 
						base_sc.angle = 0;
						memory[mem_idx][0] = base_sc;
						memory[mem_idx][1] = base_sc;
						memory[mem_idx][2] = base_sc;
						mem_idx++;
					}
					memorize = true;
					printf("%s : memorize on\n",__FUNCTION__);
				}
				continue;
			}
			if (js.Btn_Led & 0x2) {
				if (replay) {
					replay = false;
				} else {
					replay = true;
				}
				printf("%s replay %d\n",__FUNCTION__,replay);
			}
			
			if (replay) {
				for (auto &mem : memory) {
					//printf("%s : replayed %d\n",__FUNCTION__,mem.first);
					base.write(mem.second[0]);
					shoulder.write(mem.second[1]);
					elbow.write(mem.second[2]);
					// wait for commands to finish
					while (!elbow.empty() ||
					       !base.empty()  ||
					       !shoulder.empty() ||
					       !wrist.empty()) {
						// printf("here x %d %d %d %d\n",
						//         !elbow.empty() ,
						//         !base.empty()  ,
						//         !shoulder.empty() ,
						//         !wrist.empty());
						// ignore joy stick while being processed
						if (!jsd.empty()) js = jsd.read();
						usleep(MAX(shoulder_sc.delay,MAX(elbow_sc.delay,base_sc.delay)));
					}
					//usleep(10000);
				}
				replay = false;
			} else {
				bool shoulder_m = false, elbow_m = false;
				// follow joy stick
				base_sc.mode 	 = RELATIVE;
				base_sc.angle 	 = get_ainc(js.X,ARRAY_SIZE(angle_table_x),4,angle_table_x);
				base_sc.incr  	 = 1;
				base_sc.delay 	 = 2500;
				base.write(base_sc);
				if (js.Btn_Led & 1) {
					shoulder_sc.mode     = RELATIVE;
					shoulder_sc.angle    = get_ainc(js.Y,ARRAY_SIZE(angle_table_y),2,angle_table_y);
					shoulder_sc.incr     = 1;
					shoulder_sc.delay    = 10000;
					shoulder.write(shoulder_sc);
					shoulder_m = true;
				} else {
					elbow_sc.mode 	  = RELATIVE;
					elbow_sc.angle 	  = get_ainc(js.Y,ARRAY_SIZE(angle_table_y),2,angle_table_y);
					elbow_sc.incr  	  = 1;
					elbow_sc.delay 	  = 10000;
					elbow.write(elbow_sc);
					elbow_m = true;
				}
				if (memorize) {
					if (base_sc.angle ||
					    (shoulder_m && shoulder_sc.angle) ||
					    (elbow_m    && elbow_sc.angle)) {
						// printf("%s : memorized %d %d %d %d\n",__FUNCTION__, mem_idx,
						//        base_sc.angle, shoulder_sc.angle, elbow_sc.angle);
						if (!shoulder_m) shoulder_sc.angle = 0;
						if (!elbow_m)    elbow_sc.angle    = 0;
						memory[mem_idx][0] = base_sc;
						memory[mem_idx][1] = shoulder_sc;
						memory[mem_idx][2] = elbow_sc;
						mem_idx++;
					}
				}
			}
			// wait for commands to finish
			while (!elbow.empty() ||
			       !base.empty()  ||
			       !shoulder.empty() ||
			       !wrist.empty()) {
				// printf("here x %d %d %d %d\n",
				//         (elbow_w && !elbow.empty()) ,
				//         (base_w  && !base.empty())  ,
				//         (shoulder_w && !shoulder.empty()) ,
				//         (wrist_w && !wrist.empty()));
				// ignore joy stick while being processed
				if (!jsd.empty()) js = jsd.read();
				usleep(MAX(shoulder_sc.delay,MAX(elbow_sc.delay,base_sc.delay)));
			}
                }
		usleep(10);
	}
}

