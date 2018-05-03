#include <stdio.h>
#include <hls_stream.h>

// will write an integer everytime the block is woken up
void timer_controlled_block(hls::stream<int> &outs)
{
	static int count = 0;
	outs.write(count++);
}


// receive integer and print the value
void timer_receive(hls::stream<int> &ins)
{
	while (1) {
		int count = ins.read();
		printf("%s: received %d\n",__FUNCTION__, count);
	}
}
