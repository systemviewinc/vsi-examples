import java.io.*;
import java.util.Arrays;
import com.systemviewinc.runtime.*;

// Must match the C side definition
public class joy_stick {
	public short X;
	public short Y;
	public char  Btn_Led;
	public Buffer toBuffer() {
		Buffer bb = new Buffer(5);
		bb.putShort(X);
		bb.putShort(Y);
		bb.putChar(Btn_Led);
		bb.rewind();
		return bb;
	}
	public void fromBuffer(Buffer bb) {
		bb.rewind();
		X = bb.getShort();
		Y = bb.getShort();
		Btn_Led = bb.getChar();
	}
	public joy_stick() {
		X =0 ;
		Y =0;
		Btn_Led = 0;
	}
}
