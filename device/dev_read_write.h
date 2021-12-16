#ifndef __DEV_READ_WRITE_H
#define __DEV_READ_WRITE_H


#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define ABS(x) 	      (x < 0 ? -x : x)
#define MAX(x,y)      (x > y ?  x : y)

enum modes {
	NORMAL   = 0,
	RELATIVE = 1,
	MEM_POS  = 2,
	POS_MEM  = 3
};

typedef struct servo_command {
	int 	mode;  	   // command contains relative changes to the angle
	int 	angle;     // destination angle
	int 	incr;      // angle increment
	int 	delay;     // delay uS between each
} servo_command ;

// joystick entry to angle increment table
class angle_table {
	short *min, *max;
	float *i_angle;
	int n_entries;
 public:
	angle_table (short j_min, short j_max, int size, int ai_max) {
		int n_entries = size;
		float c_angle = ai_max;
		short incr    = (j_max - j_min)/n_entries;
		short c_min   = j_min;
		short c_max   = j_min + incr;
		i_angle = new float [n_entries];
		min     = new short [n_entries];
		max     = new short [n_entries];

		for (int i = 0 ; i < n_entries; i++) {
			min[i]     = c_min;
			max[i]     = c_max;
			i_angle[i] = c_angle;
			c_min = c_max;
			c_max += incr;
			c_angle -= (2*(float)ai_max)/n_entries;
		}
	}
	float get_ainc(short val, int sens) {
		int a_size = n_entries;
		for (int i = 0 ; i < a_size; i++) {
			if (val >= min[i] && val <= max[i]) {
				// de-sensitize the center of the joy stick
				if (i >= (a_size/2) - sens &&
				    i <= (a_size/2) + sens) return 0.0;
				return i_angle[i];
			}
		}
		// out of range
		return 0.0;
	}
	~angle_table() {
		delete [] i_angle;
		delete [] min;
		delete [] max;
	}
};

template<int min_pw,int max_pw,int pc,int max_deg> void servo_motor(hls::stream<servo_command> &s_cmd,
								    vsi::device<int> &atm);

typedef struct __attribute__ ((packed)) joy_stick {
	short X;
	short Y;
	char  Btn_Led;
} js_data;

void servo_HS55(hls::stream<servo_command> &s_cmd, vsi::device<int> &mot);

#endif
