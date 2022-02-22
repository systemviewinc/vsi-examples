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

	public static void process_device(VsiDevice dev) {
		Buffer in = new Buffer(256);
		in.fill("abcd0123");
		Buffer out = new Buffer(256);
		dev.pwrite(in, 0);
		dev.pread(out, 0);
		boolean is_same = in.compare(out);
		System.out.println("buffers are same: " + is_same);
	}

	public static void write_stream(StreamBuffer sb) {
		Buffer in = new Buffer(4);
		in.fill("abcd");
		sb.write(in);
	}
}
