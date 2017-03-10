count = 0


def process(bufIn, bufOut):
    global count
    count += 1
    bufOut.putInt(0x4543484f)
    bufOut.putShort(0x212e)
    bufOut.putChar(0x0A)
    bufOut.putChar(0x0B)
    bufOut.putChar(0x0C)
    bufOut.putChar(0x0D)
    bufOut.put(bufIn, 10)


def process_stream(sbIn, sbOut):
    global count
    bufIn = sbIn.read()
    bufOut = Buffer(256)
    process(bufIn, bufOut)
    if ((count % 100000) == 0):
        print("count is " + count)
        print(bufIn)
        print(bufOut)
    sbOut.write(bufOut)
