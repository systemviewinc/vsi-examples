#include <hls_stream.h>
#include <ap_utils.h>
#include "ap_axi_sdata.h"
#ifndef __VSI_HLS_SYN__
#include <vsi_device.h>
#include <iostream>
#endif

#define N_SAMPLES 16
#define DATA_BIT_WIDTH 32
#define DATA_BYTE_WIDTH DATA_BIT_WIDTH/8

#ifndef __VSI_HLS_SYN__
// PS only functions

void printApDatBytes(ap_uint<32> dat)
{
    ap_uint<8> bytes[4];
    for(int i = 0; i < 4; ++i){
        bytes[i] = (dat >> i*8) & 0xff;
    }
    for(int i = 3; i >= 0; --i){
        std::cout << std::hex << bytes[i] << " ";
    }
}

void printApDatBytes(ap_uint<128> dat)
{
    ap_uint<8> bytes[16];
    for(int i = 0; i < 16; ++i){
        bytes[i] = (dat >> i*8) & 0xff;
    }
    for(int i = 15; i >= 0; --i){
        std::cout << std::hex << bytes[i] << " ";
    }
}

template <typename T>
void printDat(T dat)
{
    printApDatBytes(dat);
    std::cout << ", as decimal = " << std::dec << dat;
}

void driver(hls::stream<ap_uint<DATA_BIT_WIDTH> >& instream0, 
    hls::stream<ap_uint<DATA_BIT_WIDTH> >& outstream0,
    hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& instream1,
    hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& outstream1,
    vsi::device<int>& mem)
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
    exit(0); // Exit ps application.
}
#endif

// HLS process data functions

void process_stream_and_mem(hls::stream<ap_uint<DATA_BIT_WIDTH> >& instream, 
    hls::stream<ap_uint<DATA_BIT_WIDTH> >& outstream,
    ap_uint<DATA_BIT_WIDTH>* mem)
{
    ap_uint<DATA_BIT_WIDTH> buf[N_SAMPLES];
    ap_uint<DATA_BIT_WIDTH> mem_offset = 0;

    for(int i = 0; i < N_SAMPLES; i++){
        buf[i] = instream.read();
        mem[mem_offset++] = buf[i]+1000;
    }

    for(int i = 0; i < N_SAMPLES; i++){
        outstream.write(buf[i]+100);
    }
}

void process_stream_packet(hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& instream, 
    hls::stream<ap_axiu<DATA_BIT_WIDTH,0,0,0> >& outstream)
{
    ap_axiu<32,0,0,0> strm_pkt;
    for(int i = 0; i < N_SAMPLES; ++i){
        strm_pkt = instream.read();
        strm_pkt.data = strm_pkt.data*2;
        strm_pkt.keep = -1;
        outstream.write(strm_pkt);
    }
}

