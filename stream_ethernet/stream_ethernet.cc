#include "stream_ethernet.h"
#include "ws_packets.h"

//#define ARB_ON_LAST

/**
 * @brief generate 1-byte ethernet frames
 *
 * @param outp  byte stream frame output
 */

void ethernet_frame_generator(hls::stream<ap_axis_dk<8>> &outp, vsi::device &ethernet_core_control, vsi::device &ethernet_core_status)
{ 
  struct packet_array {
    unsigned int *pkt;
    int pkt_size;
  };
  const int MAX_PACKETS = 42;
  struct packet_array pkt_arr[MAX_PACKETS] = {
    {pkt1, sizeof(pkt1)}, 
    {pkt2, sizeof(pkt2)},
    {pkt3, sizeof(pkt3)}, 
    {pkt4, sizeof(pkt4)},
    {pkt5, sizeof(pkt5)},
    {pkt6, sizeof(pkt6)},
    {pkt7, sizeof(pkt7)},
    {pkt8, sizeof(pkt8)},
    {pkt9, sizeof(pkt9)},
    {pkt10, sizeof(pkt10)},
    {pkt11, sizeof(pkt11)},
    {pkt12, sizeof(pkt12)},
    {pkt13, sizeof(pkt13)},
    {pkt14, sizeof(pkt14)},
    {pkt15, sizeof(pkt15)},
    {pkt16, sizeof(pkt16)},
    {pkt17, sizeof(pkt17)},
    {pkt18, sizeof(pkt18)},
    {pkt19, sizeof(pkt19)},
    {pkt20, sizeof(pkt20)},
    {pkt21, sizeof(pkt21)},
    {pkt22, sizeof(pkt22)},
    {pkt23, sizeof(pkt23)},
    {pkt24, sizeof(pkt24)},
    {pkt25, sizeof(pkt25)},
    {pkt26, sizeof(pkt26)},
    {pkt27, sizeof(pkt27)},
    {pkt28, sizeof(pkt28)},
    {pkt29, sizeof(pkt29)},
    {pkt30, sizeof(pkt30)},
    {pkt31, sizeof(pkt31)},
    {pkt32, sizeof(pkt32)},
    {pkt33, sizeof(pkt33)},
    {pkt34, sizeof(pkt34)},
    {pkt35, sizeof(pkt35)},
    {pkt36, sizeof(pkt36)},
    {pkt37, sizeof(pkt37)},
    {pkt38, sizeof(pkt38)},
    {pkt39, sizeof(pkt39)},
    {pkt40, sizeof(pkt40)},
    {pkt41, sizeof(pkt41)},
    {pkt42, sizeof(pkt42)}};
    
  const int NUM_PACKETS = 2; // change this to transmit required number of packets, NUM_PACKES maximum is MAX_PACKETS

  uint32_t rval = 0;

  uint32_t wval = 0x00000000;
  printf("In %s, ethernet core in NON-loopback (normal) mode..\n",__FUNCTION__);
  ethernet_core_control.pwrite(&wval, sizeof(wval), 0);


  printf("Waiting for stat_rx_status to go high...\n");
  while(!(rval &0x01)) {
      ethernet_core_status.pread(&rval, sizeof(rval), 0);
  }
  printf("stat_rx_status is 1, transmitting frames...\n");

  for (int i = 0; i < NUM_PACKETS; i++) {
    printf("Transmitting packet %d...\n",i+1);
    int pkt_len = (pkt_arr[i].pkt_size)/sizeof(unsigned int);
    for(int j = 0; j < pkt_len;j++) {
      ap_axis_dk<8> e;
      e.data = pkt_arr[i].pkt[j];
      e.keep = -1;
      e.last = (j == (pkt_len - 1));      	
      outp.write(e);
    }
  }
  printf("Done transmitting %d packets..\n", NUM_PACKETS);

  while(1) {
    ;
  }
}

/**
 * @brief converts 8-bit data into 64-bits
 *
 * @param insLast
 * @param outsLast
 */
void pass_thru_last(hls::stream<ap_axis_dk<8> > &ins8,
		    hls::stream<ap_axis_dk<DATA_WIDTH> > &outs64)
{
  const int MAXSIZE = 1514;
  uint8_t buffer[MAXSIZE];
  uint64_t buffer64[MAXSIZE];
  int length = 0;


  for(int i = 0;i < MAXSIZE;i++) {
    buffer[i] = 0;
    buffer64[i] = 0;
  }
  
  // read in 8-bit data into an 8-bit buffer
  while(!(ins8.empty())) {
    ap_axis_dk<8> e;
    e = ins8.read();
    buffer[length++] = e.data;
  }

  length += (length & 7); // byte align

  int buf64_pos = 0;
  uint64_t packet64;
  // store into 64-bit buffer
  for(int buf_pos = 0; buf_pos < length;buf_pos+=8) {
    /* @TODO (bharath):
     * It seems like Vivado HLS is not unfolding the loop as expected (regardless of pipelining)
     * The simulation behavior is that the first 64-bits (8 bytes) of the packet
     * are correct, rest are all zeros.  In the simulator, examining the associated
     * D flip-flop based memory shows the first memory location with the correct 64-bit value,
     * rest are all 0.
     * So, using a brute force approach.
     * Note that the code below has the correct behavior in traditional C
     */
    /*
 #pragma HLS PIPELINE II=1        
    packet64 = 0;
    for(int k = buf_pos;k < (buf_pos+8);k++) {
      packet64 <<= 8;
      packet64 |= ((uint64_t) buffer[k]);
      }
    */


    packet64 = 0;
    int byte0pos = buf_pos+0;
    int byte1pos = buf_pos+1;
    int byte2pos = buf_pos+2;
    int byte3pos = buf_pos+3;
    int byte4pos = buf_pos+4;
    int byte5pos = buf_pos+5;
    int byte6pos = buf_pos+6;
    int byte7pos = buf_pos+7;
    packet64 = ((uint64_t) buffer[byte7pos]) |
      (((uint64_t) buffer[byte6pos]) << 8) | 
      (((uint64_t) buffer[byte5pos]) << 16) |
      (((uint64_t) buffer[byte4pos]) << 24) |
      (((uint64_t) buffer[byte3pos]) << 32) | 
      (((uint64_t) buffer[byte2pos]) << 40) | 
      (((uint64_t) buffer[byte1pos]) << 48) | 
      (((uint64_t) buffer[byte0pos]) << 56);

    buffer64[buf64_pos++] = packet64;
  }

  // transmit
  ap_axis_dk<DATA_WIDTH> f;
  for(int i = 0; i < buf64_pos;i++) {
 #pragma HLS PIPELINE II=1    
    f.data = buffer64[i];
    f.keep = -1;
    f.last = (i == (buf64_pos-1));
    outs64.write(f);
  }
}

void display_ethernet_data(hls::stream<ap_axis_dk<DATA_WIDTH> > &ins)
{
  printf("%s started\n",__FUNCTION__);
  printf("Data received:\n");
  while(!ins.empty()) {
    ap_axis_dk<DATA_WIDTH> r = ins.read();
    printf("0x%016lx\n", ((uint64_t) r.data));
    if (r.last == 1)
      break;
  }
}

