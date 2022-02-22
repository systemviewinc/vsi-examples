#ifndef RDMA_HELPER_H
#define RDMA_HELPER_H

#include <mutex>
#include <map>
#include <condition_variable>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vsi_device.h>
#include "rdma.h"

#define ERR_BUSY -1
#define ERR_NO_GRP -2 // A request was made to run a program that's not registered.
#define ERR_UNKOWN -3

enum rdma_state{init, busy, complete};

struct rdma_prog{
	unsigned int* prog = NULL;
	size_t prog_sz;
};

struct rdma_config{
	int curr_grp_id = -1; // initialize to known nonvalid group
	int curr_prog_rqst = -1; // -1 represents "no current request"
	enum rdma_state curr_rdma_state = init;
	std::map<int, struct rdma_prog> prog_table; // key = grp_id, val = prog
};

int set_tot_rdma_registrations(int tot_regis_arg);
int register_rdma_prog(std::string rdma_name, int grp_id, unsigned int* prog, size_t prog_sz);
int process_rdma_state(std::string rdma_name, vsi::device<int> &rdma_control, vsi::device<int> &program_mem);
int run_rdma_grp(int grp_id);
void print_rdma_table(); // for debug

#endif /* end of include guard: RDMA_HELPER_H */
