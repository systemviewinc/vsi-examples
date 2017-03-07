import java.io.*;
import java.nio.*;
import java.util.Arrays;

// Must match the C side definition
public class joy_stick {
	public short X;
	public short Y;
	public byte  Btn_Led;
	public ByteBuffer toByteBuffer() {
		ByteBuffer bb = Utils.makeDirectByteBuffer(5);
		Utils.toBytes(X,bb);
		Utils.toBytes(Y,bb);
		bb.put(Btn_Led);
		bb.rewind();
		return bb;
	}
	public void fromByteBuffer(ByteBuffer bb) {
		bb.rewind();
		X = Utils.makeShort(bb);
		Y = Utils.makeShort(bb);
		byte []b = new byte[1];
		bb.get(b,0,1);
		Btn_Led = b[0];
	}
	public joy_stick() {
		X =0 ;
		Y =0;
		Btn_Led = 0;
	}
}
