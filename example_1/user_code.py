import vsi_runtime

COUNT = 0


def process(buf_in, buf_out):
    global COUNT
    COUNT += 1
    buf_out.putInt(0x4543484f)
    buf_out.putShort(0x212e)
    buf_out.putChar(0x0A)
    buf_out.putChar(0x0B)
    buf_out.putChar(0x0C)
    buf_out.putChar(0x0D)
    buf_out.put(buf_in, 10)


def process_stream(sbIn, sbOut):
    global COUNT
    buf_in = sbIn.read()
    buf_out = vsi_runtime.Buffer(256)
    process(buf_in, buf_out)
    if (COUNT % 100000) == 0:
        print COUNT
        print buf_in
        print buf_out
    sbOut.write(buf_out)


def process_device(dev):
    global COUNT
    COUNT += 1
    buf_in = vsi_runtime.Buffer(256)
    buf_out = vsi_runtime.Buffer(256)
    buf_in.fill("abcd0123")
    dev.pwrite(buf_in)
    dev.pread(buf_out)
    if  (COUNT % 100000) == 0:
        print buf_in.compare(buf_out)


def write_stream(sbOut):
    global COUNT
    buf_out = vsi_runtime.Buffer(4)
    buf_out.fill("abcd")
    sbOut.write(buf_out)
    COUNT += 1
    if (COUNT % 10) == 0:
        print COUNT
        print buf_out

def read_stream(sbIn):
    global COUNT
    buf_in = vsi_runtime.Buffer(4)
    sbIn.read(buf_in)
    if (COUNT % 10) == 0:
        print COUNT
        print buf_in
