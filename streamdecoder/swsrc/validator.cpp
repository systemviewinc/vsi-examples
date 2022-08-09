#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <zlib.h>

#include <mutex>
#include <thread>

#include "ap_axi_sdata.h"
#include "ap_int.h"
#include <ap_utils.h>
#include <hls_stream.h>
#include <string.h>

#ifndef __VSI_HLS_SYN__
#include <unistd.h>
#include <vsi_device.h>
#endif
#include "crc32.h"

using namespace std;

#define TEST_DATA_SIZE 32
#define RNDSEED 42

const int constVal = 0xA5BE7312;

bool terminate_thr = false;

enum errors
{
  TX_TIMEOUT_ERROR = 150,
  RX_TIMEOUT_ERROR,
  INTEGRITY_ERROR
};

struct ProcessInfo {
  CRC32 checksum;
  size_t data_size = 0;
  bool status = false;
};

void txdata(hls::stream<int> *codestream, 
            hls::stream<int> *codeword,
            size_t n_semples,
            ProcessInfo *processinfo) {
  unsigned int data = 0;
  codeword->write(&constVal, sizeof(constVal));
  srand (RNDSEED); // Make data repeateble
  while (n_semples--) {
    data = rand() % INT_MAX;
    // calculate checksum of uncoded data
    processinfo->checksum.add((const void *)&data, sizeof(data));
    // Send masked data and let HLS decode it
    data ^= constVal;
    codestream->write(&data, sizeof(data));
    processinfo->data_size++;
  }

  processinfo->status = true;

  while (!terminate_thr) { };
}

void rxdata(hls::stream<int> *dcdstream, ProcessInfo *processinfo) {
  unsigned int data = 0;
  while (!terminate_thr) {
    data = dcdstream->read();
    processinfo->checksum.add((const void *)&data, sizeof(data));
    processinfo->data_size++;
  }
}

void data_validation ( ProcessInfo *tostream, ProcessInfo *fromstream )
{
  unsigned long wait_iterations; //iteration counter

  wait_iterations = 100;
  while (!tostream->status) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!wait_iterations) // 'timeout'
    {
      std::cerr << "TX timout. Transmitted: " << tostream->data_size << "\n";
      exit(TX_TIMEOUT_ERROR);
    }

    wait_iterations--;
  };

  // Add timeout....
  wait_iterations = 100;
  while (tostream->data_size != fromstream->data_size) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!wait_iterations) // 'timeout'
    {
      std::cerr << "RX timout. Received: " << fromstream->data_size << "\n";
      exit(RX_TIMEOUT_ERROR);
    }
    wait_iterations--;
  };

  if (tostream->checksum.getHash() != fromstream->checksum.getHash()) {
    std::cerr << "Data integrity error\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    exit(INTEGRITY_ERROR);
  }
  terminate_thr = true;
  std::cout << "Done\n";
  std::this_thread::sleep_for(std::chrono::seconds(1));

  exit(0);
}

/**
 * @brief Generation of stream data, transmit to codestream stream 
 *        read it back and check if data are the same
 *
 * @param dcdstream - decoded input stream
 * @param codeword - codeword output int value
 * @param codestream - encoded output stream
 */
void stream_echo_validation(
  hls::stream<int> &dcdstream,
  hls::stream<int> &codeword,
  hls::stream<int> &codestream)
{
  ProcessInfo tostream;
  ProcessInfo fromstream;

  thread write_th(txdata, &codestream, &codeword, TEST_DATA_SIZE,
                  &tostream);                // Write thread declaration
  thread read_th(rxdata, &dcdstream, &fromstream); // Read thread declaration
  thread data_validation_th(data_validation, &tostream,
                            &fromstream); // Read thread declaration
  data_validation_th.join();
  write_th.join();
  read_th.join();
}
