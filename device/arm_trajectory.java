import java.io.*;
import java.nio.*;
import java.util.Arrays;
import com.systemviewinc.runtime.*;

class Main {
	// Must match the C side definition
	public class servo_command implements java.io.Serializable {
		public int 	mode;  	   // command contains relative changes to the angle
		public int 	angle;     // destination angle
		public int 	incr;      // angle increment
		public int 	delay;     // delay uS between each
		public byte[] to_bytes () {
			ByteArrayOutputStream bos = new ByteArrayOutputStream();
			ObjectOutputStream outs = new ObjectOutputStream(bos);
			outs.writeInt(mode);
			outs.writeInt(angle);
			outs.writeInt(incr);
			outs.writeInt(delay);
			outs.close();
			byte[] rv = bos.toByteArray();
			bos.close();
			return rv;
		}
		servo_command(byte[] data) {
			ByteArrayInputStream in = new ByteArrayInputStream(data);
			ObjectInputStream oin = new ObjectInputStream(in);
			mode  = oin.readInt();
			angle = oin.readInt();
			incr  = oin.readInt();
			delay = oin.readInt();
			oin.close();
			in.close();
		}
		public ByteBuffer toByteBuffer() {
			ByteArrayOutputStream bos = new ByteArrayOutputStream();
			ObjectOutputStream outs = new ObjectOutputStream(bos);
			outs.writeInt(mode);
			outs.writeInt(angle);
			outs.writeInt(incr);
			outs.writeInt(delay);
			outs.close();
			byte[] rv = bos.toByteArray();
			bos.close();
			return ByteBuffer.wrap(rv);
		}
		servo_command(ByteBuffer inbb) {
			byte[] data = new byte[inbb.remaining()];
			inbb.get(data);
			ByteArrayInputStream in = new ByteArrayInputStream(data);
			ObjectInputStream oin = new ObjectInputStream(in);
			mode  = oin.readInt();
			angle = oin.readInt();
			incr  = oin.readInt();
			delay = oin.readInt();
			oin.close();
			in.close();		
		}
	}

	static public void test_servo_command(StreamBuffer in_sc, StreamBuffer out_sc) {
		while (true) {
			servo_command sc = new servo_command((ByteBuffer)in_sc.readByteBuffer());
			System.out.println("Got servo command angle " + sc.angle +
					   " mode " + sc.mode + " incr " + sc.incr + " delay " + sc.delay);
			sc.angle += 10;
			sc.incr  += 10;
			sc.delay += 10;
			out_sc.writeByteBuffer(sc.toByteBuffer());
		}
	}
	
}	
