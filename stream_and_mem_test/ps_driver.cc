#include "stream_and_mem_test.h"

#ifndef __VSI_HLS_SYN__
// PS only functions

void printDat(ap_uint<DATA_BIT_WIDTH> dat)
{
    ap_uint<8> bytes[DATA_BYTE_WIDTH];
    for(int i = 0; i < DATA_BYTE_WIDTH; ++i){
        bytes[i] = (dat >> i*8) & 0xff;
    }
    for(int i = DATA_BYTE_WIDTH-1; i >= 0; --i){
        std::cout << std::hex << bytes[i] << " ";
    }
    std::cout << ", as decimal = " << std::dec << dat;
}

void driver(hls::stream<ap_uint<DATA_BIT_WIDTH> >& instream0, 
    hls::stream<ap_uint<DATA_BIT_WIDTH> >& outstream0,
    hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& instream1,
    hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& outstream1,
    vsi::device<ap_uint<DATA_BIT_WIDTH> >& mem)
{
    ap_uint<DATA_BIT_WIDTH> i, dat;
    int err_count = 0;

    std::cout << "---- Stream and Mem Test ----" << std::endl;

    for(i = 0; i < N_SAMPLES; ++i){
        std::cout << "Send to outstream0: bytes = ";
        printDat(i);
        std::cout << std::endl;
        outstream0.write(i);
    }

    std::cout << "Readback processed data. Expect sent data+100" << std::endl;
    for(i = 0; i < N_SAMPLES; ++i){
        dat = instream0.read();
        std::cout << "Read from instream0: bytes = ";
        printDat(dat);
        if(dat != i+100){
            ++err_count;
            std::cout << " <ERROR: expected " << i+100;
        }
        std::cout << std::endl;
    }

    std::cout << "Readback processed data. Expect sent data+1000" << std::endl;
    int offset = 0;
    for(i = 0; i < N_SAMPLES; ++i){
        mem.pread(&dat, DATA_BYTE_WIDTH, offset);
        offset += DATA_BYTE_WIDTH;
        std::cout << "Read from mem: bytes = ";
        printDat(dat);
        if(dat != i+1000){
            ++err_count;
            std::cout << " <ERROR: expected " << i+1000;
        }
        std::cout << std::endl;
    }

    std::cout << "---- Stream Packet Test ----" << std::endl;

    ap_axiu<DATA_BIT_WIDTH,0,0,0> strm_pkt;
    for(i = 0; i < N_SAMPLES; ++i){
        strm_pkt.data = i;
        if(i == N_SAMPLES-1){
            strm_pkt.last = 1;
        }
        else{
            strm_pkt.last = 0;
        }
        std::cout << "Send to outstream1: bytes = ";
        printDat(strm_pkt.data);
        std::cout << std::endl;
        outstream1.write(strm_pkt);
    }

    std::cout << "Readback processed data. Expect sent data*2" << std::endl;
    for(i = 0; i < N_SAMPLES; ++i){
        strm_pkt = instream1.read();
        std::cout << "Read from instream1: bytes = ";
        printDat(strm_pkt.data);
        if(strm_pkt.data != i*2){
            ++err_count;
            std::cout << " <ERROR: expected " << i*2;
        }
        std::cout << std::endl;
    }

    if(err_count){
        std::cout << "ERROR: corrupt data read back!" << std::endl;
    }
    else{
        std::cout << "SUCCESS: valid data read back!" << std::endl;
    }
    exit(0); // Exit PS application.
}

#endif

