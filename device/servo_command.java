import java.io.*;
import java.util.Arrays;
import com.systemviewinc.runtime.*;


// Must match the C side definition
public class servo_command {
	public enum modes {
		NORMAL,
		RELATIVE,
		MEM_POS,
		POS_MEM
	}
	public modes 	mode;  	   // command contains relative changes to the angle
	public int 	angle;     // destination angle
	public int 	incr;      // angle increment
	public int 	delay;     // delay uS between each
	public String mode_toString() {
		switch(mode) {
		case NORMAL:   return "NORMAL"   ;
		case RELATIVE: return "RELATIVE" ;
		case MEM_POS:  return "MEM_POS"  ;
		case POS_MEM:  return "POS_MEM"  ;
		}
		return "NONE";
	}
	public servo_command () {
		mode = modes.NORMAL;
	}
	public servo_command (modes c_mode, int c_angle, int c_incr, int c_delay) {
		mode = c_mode;
		angle = c_angle;
		incr = c_incr;
		delay = c_delay;
	}
	public  servo_command (servo_command sc) {
		this (sc.mode, sc.angle, sc.incr, sc.delay);
	}
	public Buffer toBuffer() {
		Buffer nat_bb = new Buffer(16);
		switch(mode) {
		case NORMAL:	nat_bb.putInt(0); break;
		case RELATIVE:	nat_bb.putInt(1); break;
		case MEM_POS:	nat_bb.putInt(2); break;
		case POS_MEM:	nat_bb.putInt(3); break;
		}

		nat_bb.putInt(angle);
		nat_bb.putInt(incr);
		nat_bb.putInt(delay);
		nat_bb.rewind();
		return nat_bb;

	}
	public String toStr () {
		return " mode:" + mode_toString() +
			" angle:" + Integer.toString(angle) +
			"incr:"  + Integer.toString(incr) +
			"delay:" + Integer.toString(delay);
	}
	public void fromBuffer(Buffer inbb) {
		inbb.rewind();
		switch(inbb.getInt()) {
		case 0: mode = modes.NORMAL; break;
		case 1: mode = modes.RELATIVE; break;
		case 2: mode = modes.MEM_POS; break;
		case 3: mode = modes.POS_MEM; break;
		}
		angle = inbb.getInt();
		incr  = inbb.getInt();
		delay = inbb.getInt();
	}
}
