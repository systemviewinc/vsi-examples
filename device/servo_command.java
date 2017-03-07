import java.io.*;
import java.nio.*;
import java.util.Arrays;


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
	public ByteBuffer toByteBuffer() {
		ByteBuffer nat_bb = Utils.makeDirectByteBuffer(16);
		switch(mode) {
		case NORMAL:	Utils.toBytes(0,nat_bb); break;
		case RELATIVE:	Utils.toBytes(1,nat_bb); break;
		case MEM_POS:	Utils.toBytes(2,nat_bb); break;
		case POS_MEM:	Utils.toBytes(3,nat_bb); break;
		}

		Utils.toBytes(angle,nat_bb);
		Utils.toBytes(incr,nat_bb);
		Utils.toBytes(delay,nat_bb);
		nat_bb.rewind();
		return nat_bb;
		
	}
	public String toStr () {
		return " mode:" + mode_toString() +
			" angle:" + Integer.toString(angle) +
			"incr:"  + Integer.toString(incr) +
			"delay:" + Integer.toString(delay);
	}
	public void fromByteBuffer(ByteBuffer inbb) {
		inbb.rewind();
		switch(Utils.makeInt(inbb)) {
		case 0: mode = modes.NORMAL; break;
		case 1: mode = modes.RELATIVE; break;
		case 2: mode = modes.MEM_POS; break;
		case 3: mode = modes.POS_MEM; break;
		}
		angle = Utils.makeInt(inbb);
		incr  = Utils.makeInt(inbb);
		delay = Utils.makeInt(inbb);
	}
}
