import java.io.*;
import java.nio.*;
import java.util.*;
import java.util.concurrent.*;
import com.systemviewinc.runtime.*;

class Main {
	static servo_command sc = new servo_command() ;
	static ByteBuffer sc_recv_b = Utils.makeDirectByteBuffer(32);
	static void usleep(int usecs) {
		try {
			TimeUnit.MICROSECONDS.sleep(usecs);
		} catch (Exception e) {};
	}
	static public void test_servo_command(StreamBuffer out_sc) {
		System.out.println("Java thread started");
		try {
			while (true) {
				sc.mode  = servo_command.modes.MEM_POS;
				sc.angle = 99;
				sc.incr  = 99;
				sc.delay = 99;
				out_sc.writeByteBuffer(sc.toByteBuffer());
				usleep(100);
			}
		} catch (Exception e) {
			System.out.println(" here " + e);
		}
	}
	static angle_table angle_table_x = new angle_table((short)190,(short)900,25);
	static angle_table angle_table_y = new angle_table((short)160,(short)900,100);
	
	static joy_stick  js  = new joy_stick();
	static ByteBuffer js_recv_b = Utils.makeDirectByteBuffer(16);
	static servo_command base_sc = new servo_command();
	static servo_command shoulder_sc = new servo_command();
	static servo_command elbow_sc = new servo_command();
	static servo_command wrist_sc = new servo_command();

	static TreeMap<Integer,servo_command[]> memory = new TreeMap<Integer,servo_command[]>();
	static public void trajectory_generator(StreamBuffer jsd,
						StreamBuffer base,
						StreamBuffer shoulder,
						StreamBuffer elbow,
						StreamBuffer wrist,
						StreamBuffer pump) {
		boolean memorize = false;
		boolean replay = false;
		int mem_idx = 0;
		servo_command [] sc_ar ;
		try {
			while (true) {
				// joy stick in control
				while (!jsd.empty()) {
					jsd.copyByteBuffer(js_recv_b);
					js.fromByteBuffer(js_recv_b);
					//System.out.println("Joy stack data "
					//		   + " " + js.X
					//		   + " " + js.Y 
					//		   + " " + Integer.toHexString((int)js.Btn_Led));
					// memorize button pressed
					if ( ((int)js.Btn_Led & (int)0x04) != 0) {
						// in memorize mode already : complete memorize
						if (memorize) {
							memorize = false;
							// memorize position end
							base_sc.mode = servo_command.modes.POS_MEM;
							base_sc.angle = 0;
							shoulder.writeByteBuffer(base_sc.toByteBuffer());
							elbow.writeByteBuffer(base_sc.toByteBuffer());
							base.writeByteBuffer(base_sc.toByteBuffer());
							wrist.writeByteBuffer(base_sc.toByteBuffer());
							System.out.println("memorize off");
						} else {						
							memory.clear(); // clear memory
							base_sc.mode = servo_command.modes.MEM_POS; // memorize current location
							base_sc.angle = 0;
							shoulder.writeByteBuffer(base_sc.toByteBuffer());
							elbow.writeByteBuffer(base_sc.toByteBuffer());
							base.writeByteBuffer(base_sc.toByteBuffer());
							wrist.writeByteBuffer(base_sc.toByteBuffer());
							System.out.println("memorize on");
							memorize = true;
							
							mem_idx = 0;
							// begin memory by positioning to location
							base_sc.mode = servo_command.modes.POS_MEM;
							base_sc.angle = 0;
							sc_ar  = new servo_command[4];
							sc_ar[0] = new servo_command(base_sc);
							sc_ar[1] = new servo_command(base_sc);
							sc_ar[2] = new servo_command(base_sc);
							sc_ar[3] = new servo_command(base_sc);
							memory.put(mem_idx,sc_ar);
							mem_idx++;
						}
						continue;
					}
					if (((int)js.Btn_Led & 0x2) != 0) {
						if (replay) replay = false;
						else replay = true;
						System.out.println("replay " + replay);
					}
					if (replay) {
						int i = 0;
						Set set = memory.entrySet();
						Iterator mi = set.iterator();						
						while(mi.hasNext()) {							
							Map.Entry me = (Map.Entry)mi.next();
							sc_ar = memory.get(me.getKey());
							base.writeByteBuffer(sc_ar[0].toByteBuffer());
							shoulder.writeByteBuffer(sc_ar[1].toByteBuffer());
							elbow.writeByteBuffer(sc_ar[2].toByteBuffer());
							wrist.writeByteBuffer(sc_ar[3].toByteBuffer());
							// System.out.println("replaying " + i++
							// 		   + " [0] " + sc_ar[0].toStr()
							// 		   + " [1] " + sc_ar[1].toStr()
							// 		   + " [2] " + sc_ar[2].toStr()
							// 		   + " [3] " + sc_ar[3].toStr()	
							// 	   );
							// while (!base.ready() ||
							//        !shoulder.ready() ||
							//        !elbow.ready() ||
							//        !wrist.ready()) {
							// 	if (!jsd.empty()) jsd.copyByteBuffer(js_recv_b);
							// 	// System.out.println("waiting " +
							// 	// 		   " " + base.ready() +
							// 	// 		   " " + shoulder.ready() +
							// 	// 		   " " + elbow.ready() +
							// 	// 		   " " + wrist.ready());
							// 	usleep(1000);
							// }
						}
						System.out.println("Replay complete");
						replay = false; // replay complete
						continue;
					} else {
						// follow joystick
						boolean button = false;
						if (((int)js.Btn_Led & 1) != 0) {
							shoulder_sc.mode  = servo_command.modes.RELATIVE;
							shoulder_sc.angle = (int)angle_table_y.get_ainc(js.Y,2);
							shoulder_sc.incr  = 1;
							shoulder_sc.delay = 10000;
							wrist_sc.mode  	  = servo_command.modes.RELATIVE;
							wrist_sc.angle    = (int)angle_table_x.get_ainc(js.X,4);
							wrist_sc.incr     = 1;
							wrist_sc.delay    = 2500;
							shoulder.writeByteBuffer(shoulder_sc.toByteBuffer());
							wrist.writeByteBuffer(wrist_sc.toByteBuffer());
							button = false;
						} else {
							elbow_sc.mode 	  = servo_command.modes.RELATIVE;
							elbow_sc.angle 	  = (int)angle_table_y.get_ainc(js.Y,2);
							elbow_sc.incr  	  = 1;
							elbow_sc.delay 	  = 10000;
							elbow.writeByteBuffer(elbow_sc.toByteBuffer());
							base_sc.mode 	  = servo_command.modes.RELATIVE;
							base_sc.angle 	  = (int)angle_table_x.get_ainc(js.X,4);
							base_sc.incr  	  = 1;
							base_sc.delay 	  = 2500;
							base.writeByteBuffer(base_sc.toByteBuffer());
							button = false;
						}
						if (memorize) {
							if ((button && (shoulder_sc.angle != 0 || wrist_sc.angle != 0)) ||
							    (!button && (base_sc.angle != 0 || elbow_sc.angle != 0))) {
								if (!button) {
									shoulder_sc.angle = 0;
									wrist_sc.angle = 0;
								} else {
									elbow_sc.angle = 0;
									base_sc.angle  = 0;
								}
								sc_ar = new servo_command[4];
								sc_ar[0] = new servo_command(base_sc);
								sc_ar[1] = new servo_command(shoulder_sc);
								sc_ar[2] = new servo_command(elbow_sc);
								sc_ar[3] = new servo_command(wrist_sc);
								memory.put(mem_idx++,sc_ar);
								// System.out.println("memory index " + mem_idx
								// 		   + " [0] " + sc_ar[0].toStr()
								// 		   + " [1] " + sc_ar[1].toStr()
								// 		   + " [2] " + sc_ar[2].toStr()
								// 		   + " [3] " + sc_ar[3].toStr()
								// 		   );
							}
						}
					}
					// while (!base.ready() ||
					//        !shoulder.ready() ||
					//        !elbow.ready() ||
					//        !wrist.ready()) {
					// 	if (!jsd.empty()) jsd.copyByteBuffer(js_recv_b);
					// 	usleep(1000);
					// }
					usleep(10);
				}
				usleep(10);
			}
		} catch (Exception e) {
			System.out.println("Exception " + e);
		}
	}
}	
