import java.io.*;
import java.nio.*;
import java.util.Arrays;

public class angle_table {
	short [] min;
	short [] max;
	float [] i_angle;
	int at_size;
	public angle_table(short j_min, short j_max, int a_size) {
		at_size = a_size;
		min = new short[a_size];
		max = new short[a_size];
		i_angle = new float[a_size];
		float c_angle = 8;
		short incr = (short)((j_max - j_min)/a_size);
		short c_min= j_min;
		short c_max= (short)(j_min + incr);
		int i;
		for (i = 0 ; i < a_size; i++) {
			min[i] = c_min;
			max[i] = c_max;
			i_angle[i] = c_angle;
			c_min  = c_max;
			c_max  += incr;
			c_angle -= (2*8.0)/(float)a_size;
		}
	}
	public float get_ainc(short val, int sens) {
		int i;
		for (i = 0 ; i < at_size; i++) {
			if (val >= min[i] && val <= max[i]) {
				if (i >= (at_size/2) - sens && i <= (at_size/2) + sens) {
					return (float)0.0;
				}
				return i_angle[i];
			}
		}
		return (float)0.0;
	}
}
