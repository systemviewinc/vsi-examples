#ifndef __DEV_READ_WRITE_H
#define __DEV_READ_WRITE_H


#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define ABS(x) 	      (x < 0 ? -x : x)
#define MAX(x,y)      (x > y ?  x : y)

typedef struct servo_command {
	bool 	rela;  	   // command contains relative changes to the angle
	bool	memorize;  // memorize
	bool 	replay;    // play back from memory
	bool    reverse;   // replay in reverse
	int 	angle;     // destination angle
	int 	incr;      // angle increment
	int 	delay;     // delay uS between each	
} servo_command ;

template<int min_pw,int max_pw,int pc,int max_deg> void servo_motor(hls::stream<servo_command> &s_cmd,
								    vsi::device &atm);

typedef struct __attribute__ ((packed)) joy_stick {
	short X;
	short Y;
	char  Btn_Led;
} js_data;

void servo_HS55(hls::stream<servo_command> &s_cmd, vsi::device &mot);

#endif
