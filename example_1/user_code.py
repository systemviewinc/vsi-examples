#!python3

import os
import vsi_runtime

COUNT = 0
OFFSET = 0


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
        print("{}".format(COUNT))
        print("{}".format(buf_in))
        print("{}".format(buf_out))
    sbOut.write(buf_out)


def process_device(dev):
    global COUNT, OFFSET
    COUNT += 1
    buf_in = vsi_runtime.Buffer(256)
    buf_out = vsi_runtime.Buffer(256)
    buf_in.fill("abcd0123")
    dev.pwrite(buf_in, OFFSET)
    dev.pread(buf_out, OFFSET)
    if  (COUNT % 20) == 0:
        print("Matched:{}, COUNT:{}, OFFSET:{}".format(buf_in.compare(buf_out), COUNT, OFFSET))
    OFFSET += 1

file_name = 'data_in'


def create_file(file_name):
    if not os.path.exists(file_name):
        i = 256
        f = open(file_name, "wb")
        try:
            while i:
                f.write("ABCDEFGH12345678")
                i -= 1
        finally:
            f.close()


def process_device_file(dev):
    global COUNT, OFFSET, file_name
    create_file(file_name)
    buf_in = vsi_runtime.Buffer(256)
    buf_out = vsi_runtime.Buffer(256)
    file_size = os.stat(file_name).st_size
    f = open(file_name, "rb")
    try:
        buf_in.set(f.read(256))
        while file_size > (256 * COUNT):
            dev.pwrite(buf_in, OFFSET)
            dev.pread(buf_out, OFFSET)
            # if  (COUNT % 10) == 0:
            print("Matched:{}, COUNT:{}, OFFSET:{}".format(buf_in.compare(buf_out), COUNT, OFFSET))
            OFFSET += 1
            COUNT += 1
            buf_in.set(f.read(256))
    finally:
        f.close()


def write_stream(sbOut):
    global COUNT
    buf_out = vsi_runtime.Buffer(4)
    buf_out.fill("abcd")
    sbOut.write(buf_out)
    COUNT += 1
    if (COUNT % 10) == 0:
        print("{}".format(COUNT))
        print("{}".format(buf_out))

def read_stream(sbIn):
    global COUNT
    buf_in = vsi_runtime.Buffer(4)
    sbIn.read(buf_in)
    if (COUNT % 10) == 0:
        print("{}".format(COUNT))
        print("{}".format(buf_in))

def mem_test(data_in, mem):
    count = 0
    offset = 0
    data_in.wait_if_empty()
    while not (data_in.empty()):
        i = data_in.read()
        print("data read {}".format(i))
        mem.pwrite(i, offset)
        print("data write")
        out = 0
        mem.pread(out, offset)
        offset+= 4
        if (i != out):
            print("incorrect value read {}".format(out))
        if (count % 1000):
            print("wrote {} times\ncurrent offset: {}".format(count, offset))
        count += 1
