#ifndef __DEV_READ_WRITE_H
#define __DEV_READ_WRITE_H

typedef struct servo_command {
	int motor_n; // motor number (optional)
	int angle;   // destination angle
	int incr;    // angle increment
	int delay;   // delay uS between each	
} servo_command ;

template<int min_pw,int max_pw,int pc,int max_deg> void servo_motor(hls::stream<servo_command> &s_cmd,
								    vsi::device &atm);

void servo_HS55(hls::stream<servo_command> &s_cmd, vsi::device &mot);

#endif
