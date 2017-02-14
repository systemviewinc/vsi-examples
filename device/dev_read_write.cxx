
#include <unistd.h>
#include <sys/types.h>
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
	int c_angle;
	float c_p_a;
	bool replay_mode = false;
	servo_command cmd_mem[2048];
	int cmd_mem_idx = 0, cmd_mem_idx_r = 0;
	int max_tlr, min_tlr;
	bool memorize = false;
	
	c_p_a   = initialize_servo(min_pw, max_pw, pc, max_deg, init_pw, max_tlr, min_tlr, atm);
	c_angle = ia; // initialized angle

	if (m_debug)
		printf("%s : init complete %f %d %d %d\n", __FUNCTION__, c_p_a, c_angle, min_tlr, max_tlr);
	// process the commands as they arrive
	while (1) {
		bool done = false;
		servo_command sc ;

		// if in replay mode then pop from replay stack		
		if (replay_mode) {
			if (cmd_mem_idx_r < cmd_mem_idx) {
				sc = cmd_mem[cmd_mem_idx_r++];
			} else {
				replay_mode = false; // done
				cmd_mem_idx_r = 0;
				printf("Replay mode done\n");
				continue;
			}
		} else {
			sc = s_cmd.read(); // check the stream
			if (sc.replay) {
				replay_mode = true;
				printf("Replay mode started %d %d\n", cmd_mem_idx, cmd_mem_idx_r);
				continue;
			}
		}
		
		if (sc.rela) {
			if (sc.angle == 0) continue;
			if (rev)
				sc.angle = (-sc.angle) + c_angle; // relative
			else
				sc.angle = (sc.angle) + c_angle; // relative
		}
		
		// memorize
		if (sc.memorize) {
			if (cmd_mem_idx < ARRAY_SIZE(cmd_mem)) {
				printf("Memorizing %d %d\n",cmd_mem_idx,sc.angle);
				// record current location we fast path there
				if (cmd_mem_idx == 0) {
					memset(&cmd_mem[0],sizeof(cmd_mem[0]),0);
					cmd_mem[0].angle = c_angle;
					cmd_mem[0].incr  = 1;
					cmd_mem[0].delay = 10000;
					cmd_mem_idx++;
				}
				// record as absolute
				cmd_mem[cmd_mem_idx] = sc;
				cmd_mem[cmd_mem_idx].memorize = false;
				cmd_mem[cmd_mem_idx].rela     = false;
				cmd_mem[cmd_mem_idx].replay   = false;				
				cmd_mem_idx++;
			} else {
				printf("memory Exceed\n");
			}
		}
		unsigned int c_tlr, tlr_diff = sc.incr * c_p_a;		

		// offset 0x14 : TLR1
		atm.pread(&c_tlr,sizeof(c_tlr),0x14); 
		c_tlr = (unsigned int)0xffffffff - c_tlr;
		
		sc.angle = sc.angle > max_deg ? max_deg : sc.angle;
		sc.angle = sc.angle < 0 ? 0 : sc.angle;
		bool inc = (c_angle < sc.angle);
		if (m_debug)
		 	printf("%s : got command ca %d, sc.angle %d, sc.incr %d, sc.delay %d dtlr %d \n",
		 	       __FUNCTION__, c_angle, sc.angle, sc.incr, sc.delay, tlr_diff );
		
		// execute the command
		do {
			if (inc) {
				c_tlr += (tlr_diff);
				c_angle += sc.incr;
				c_angle  = c_angle > max_deg ? max_deg : c_angle;
				done = (c_angle >= sc.angle) || (c_tlr > max_tlr);
			} else {
				c_tlr -= (tlr_diff);
				c_angle -= sc.incr;
				c_angle  = c_angle < 0 ? 0 : c_angle;
				done = (c_angle <= sc.angle) || (c_tlr < min_tlr);
			}
			unsigned int w_c_tlr = (unsigned int) 0xffffffff - c_tlr;
			atm.pwrite(&w_c_tlr,sizeof(w_c_tlr),0x14);
			usleep(sc.delay);
		} while (!done);
		if (m_debug) printf("%s : c_tlr %d, c_angle %d\n",__FUNCTION__, c_tlr, c_angle);
	}
}

void servo_BASE(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<700,2300,20000,180,1500,90,false,false>(s_cmd,mot);
}

void servo_SHOULDER(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<1000,2000,20000,120,1500,60,true,false>(s_cmd,mot);
}

void servo_ELBOW(hls::stream<servo_command> &s_cmd, vsi::device &mot)
{
 	servo_motor<1000,2000,20000,120,1500,60,true,true>(s_cmd,mot);
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
				printf("Btn_Led 0x%x 0x%x \n",js.Btn_Led,jsp.Btn_Led);
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
} angle_table_x[100],  angle_table_y[100];

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
		printf(" [%d] : min 0x%x max 0x%x angle %d\n",i,
		       angle_table[i].min,
		       angle_table[i].max,
		       angle_table[i].i_angle);
	}
	
}

// get the angle increment from joystick value
static float get_ainc(short val, int a_size, struct a_table *angle_table)
{
	for (int i = 0 ; i < a_size; i++) {		
		if (val >= angle_table[i].min && val <= angle_table[i].max) {
			// de-sensitize the center of the joy stick
			if (i >= a_size - 2 &&
			    i <= a_size + 2) return 0.0;
			return angle_table[i].i_angle;
		}
	}
	// out of range
	return 0.0;
}

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
	bool memorize = false;
	bool replay   = false;
	sleep(1);
	printf("%s : started .. \n",__FUNCTION__);

	// generate angle increment table
	gen_aic(190,900, ARRAY_SIZE(angle_table_x), angle_table_x);
	gen_aic(160,900, ARRAY_SIZE(angle_table_y), angle_table_y);
	while(1) {
		
		// base controlled by joy stick
		while (!jsd.empty()) {			
			bool base_w = false, shoulder_w = false, elbow_w = false, wrist_w = false;
			js = jsd.read();
			// memorize mode
			if (js.Btn_Led & 0x4) {
				if (memorize) {
					memorize = false;
					printf("memorize off\n");
				} else {
					memorize = true;
					printf("memorize on\n");
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
				if (replay) {
					printf("%s replay start\n",__FUNCTION__);
					// send replay command to motor controllers
					base_sc.memorize  = false;
					base_sc.replay    = replay;
					base_sc.rela 	  = false;
					base_sc.reverse   = !replay;
					base_sc.angle 	  = 0;
					base_sc.incr  	  = 0;
					base_sc.delay 	  = 0;
					base.write(base_sc);				
					elbow.write(base_sc);
					shoulder.write(base_sc);
					base_w = true;
					elbow_w = true;
					shoulder_w = true;
					printf("%s replay end\n",__FUNCTION__);
					// wait for commands to finish
					while ((elbow_w && !elbow.empty()) ||
					       (base_w  && !base.empty())  ||
					       (shoulder_w && !shoulder.empty())) {
						printf("here %d %d %d\n",
						       (elbow_w && !elbow.empty()) ,
						       (base_w  && !base.empty())  ,
						       (shoulder_w && !shoulder.empty()));
						// ignore joy stick while being processed
						if (!jsd.empty()) js = jsd.read();
						usleep(MAX(shoulder_sc.delay,MAX(elbow_sc.delay,base_sc.delay)));
					}
					continue;
				}
			}
			if (replay) {
				usleep(10);
				continue;
			}

			// follow joy stick
			base_sc.memorize = memorize;
			base_sc.replay   = false;
			base_sc.rela 	 = true;
			base_sc.reverse  = false;
			base_sc.angle 	 = get_ainc(js.X,ARRAY_SIZE(angle_table_x),angle_table_x);
			base_sc.incr  	 = 1;
			base_sc.delay 	 = 10000;
			base.write(base_sc);
			base_w = true;
			if (js.Btn_Led & 1) {
				shoulder_sc.memorize = memorize;
				shoulder_sc.replay   = false;
				shoulder_sc.rela     = true;
				shoulder_sc.reverse  = false;
				shoulder_sc.angle    = get_ainc(js.Y,ARRAY_SIZE(angle_table_y),angle_table_y);
				shoulder_sc.incr     = 1;
				shoulder_sc.delay    = 10000;
				shoulder.write(shoulder_sc);
				shoulder_w = true;
			} else {
				elbow_sc.memorize = memorize;
				elbow_sc.replay   = false;
				elbow_sc.rela 	  = true;
				elbow_sc.reverse  = false;
				elbow_sc.angle 	  = get_ainc(js.Y,ARRAY_SIZE(angle_table_y),angle_table_y);
				elbow_sc.incr  	  = 1;
				elbow_sc.delay 	  = 10000;
				elbow.write(elbow_sc);
				elbow_w = true;
			}
			// wait for commands to finish
			while ((elbow_w && !elbow.empty()) ||
			       (base_w  && !base.empty())  ||
			       (shoulder_w && !shoulder.empty()) ||
			       (wrist_w && !wrist.empty())) {
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

