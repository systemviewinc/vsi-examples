import java.nio.*;
import java.util.Arrays;
import com.systemviewinc.runtime.*;

class Main {
	public static void process(ByteBuffer in, ByteBuffer out) {
		out.rewind();
		out.put((byte)0x45);
		out.put((byte)0x43);
		out.put((byte)0x48);
		out.put((byte)0x4f);
		out.put((byte)0x21);
		out.put((byte)0x2e);
		Utils.transfer(out, in);
	}

	public static void process_stream(StreamBuffer sbIn, StreamBuffer sbOut) {
		ByteBuffer in = (ByteBuffer)sbIn.readByteBuffer();
		Utils.printByteBuffer(in);
		ByteBuffer out = Utils.makeDirectByteBuffer(256);
		process(in, out);
		Utils.printByteBuffer(out);
		sbOut.writeByteBuffer(out);
	}

	public static void process_device(VsiDevice devIn, VsiDevice devOut) {

	}
}
