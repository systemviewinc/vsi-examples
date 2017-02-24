import java.nio.*;
import java.util.Arrays;

class Main {
	public static void process(ByteBuffer in, ByteBuffer out) {
		out.rewind();
		out.putChar('E');
		out.putChar('C');
		out.putChar('H');
		out.putChar('O');
		out.putChar('!');
		out.putChar('.');
		Utils.transfer(out, in);
	}
}
