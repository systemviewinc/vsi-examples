import java.util.Arrays;
import com.systemviewinc.runtime.*;

class Main {
	static int count = 0;
	public static void process(Buffer in, Buffer out) {
		count++;
		out.putInt(0x4543484f);
		out.putShort((short)0x212e);
		out.putChar((char)0x0A);
		out.putChar((char)0x0B);
		out.putChar((char)0x0C);
		out.putChar((char)0x0D);
		out.put(in, 10);
	}

	public static void process_stream(StreamBuffer sbIn, StreamBuffer sbOut) {
		Buffer in = sbIn.read();
		Buffer out = new Buffer(256);
		process(in, out);
		if ((count % 100000) == 0) {
			System.out.println("count is " + count);
			System.out.println(in);
			System.out.println(out);
		}
		sbOut.write(out);
	}

	public static void process_device(VsiDevice devIn, VsiDevice devOut) {

	}
}
