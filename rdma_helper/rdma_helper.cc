#include "rdma_helper.h"

/* Synchronization objects for rdma_table. */
std::mutex m_rdma_table;
std::condition_variable cv_wait_rqst;
std::condition_variable cv_wait_busy;
std::condition_variable cv_wait_complete;
/* Synchronization objects for prog registration accounting. */
std::mutex m_prog_regis;
std::condition_variable cv_wait_tot_regis;
std::condition_variable cv_wait_regis_count;

/* Semi-global resources. Only accessed using functions below with mutexs for thread safety.
 Defined static to confine scope to this file only. */
static std::map<std::string, struct rdma_config> rdma_table;
static int total_registrations = 0;
static int registration_count = 0;

/**
 * @brief Sets the semi-global variable total_registrations. The user should call this with
 * the number rdma programs they intend to register in their app. It should be called
 * before any calls to run_rdma_grp. run_rdma_grp will make sure the total number
 * actively registered programs is equal to what the user requested before running.
 * This way, if register_rdma_prog is called from a different thread then run_rdma_grp,
 * run_rdma_grp will still know if the programs it will run have been registered.
 *
 * @param tot_regis_arg The number rdma programs the user intends to register in their app.
 * @return error code
 */
int set_tot_rdma_registrations(int tot_regis_arg)
{
	std::lock_guard<std::mutex> lock(m_prog_regis);
	total_registrations = tot_regis_arg;
	cv_wait_tot_regis.notify_all();

	return 0; // todo: error checking
}

/**
 * @brief Registers an RDMA program in the master lookup table.
 *
 * @param rdma_name Each RDMA IP in FPGA should get a unique name, then prog is associated with this name.
 * @param grp_id RDMA programs can come in groups. This is a unique id for the group this prog belongs to.
 * Specifically, a group is a set of programs running on a set of RDMA IPs that fulfil a desired data movement.
 * @param prog Pointer to compiled RDMA program. Will be associated with rdma_name.
 * @param prog_sz Size of prog.
 * @return error code
 */
int register_rdma_prog(std::string rdma_name, int grp_id, unsigned int* prog, size_t prog_sz)
{
	struct rdma_prog new_prog;
	new_prog.prog = prog;
	new_prog.prog_sz = prog_sz;
	struct rdma_config new_config;
	std::lock_guard<std::mutex> lock1(m_rdma_table);
	/* Check if rdma_name entry already exists in table. */
	if(rdma_table.count(rdma_name)){
		rdma_table[rdma_name].prog_table[grp_id] = new_prog;
	}
	else{
		new_config.prog_table[grp_id] = new_prog;
		rdma_table[rdma_name] = new_config;
	}
	//printf("%s: rdma_name=%s grp_id=%d prog_sz=%d\n", __FUNCTION__, rdma_name.c_str(),
	//	grp_id, rdma_table[rdma_name].prog_table[grp_id].prog_sz);

	std::lock_guard<std::mutex> lock2(m_prog_regis);
	++registration_count;
	cv_wait_regis_count.notify_all();

	return 0; // todo: error checking
}

/**
 * @brief Communicates with and controls the state of an RDMA IP in the FPGA.
 * Call this function inside an RDMA driver PS block function. Will only return on errors.
 *
 * @param rdma_name Name of RDMA IP that this function will communicate with and control.
 * @param rdma_control Interface with control port on RDMA IP.
 * @param program_mem Interface with program port on RDMA IP.
 * @return error code
 */
int process_rdma_state(std::string rdma_name, vsi::device &rdma_control, vsi::device &program_mem)
{
	bool do_programming, do_run;
	unsigned int status;
	unsigned int data;
	unsigned int* prog;
	size_t prog_sz;
	while(1){
		{
			/* Access the semi-global table to find run reqests. */
			std::unique_lock<std::mutex> lock(m_rdma_table);
			while(rdma_table[rdma_name].curr_prog_rqst == -1){
				//printf("%s: waitRqst curr_prog_rqst=%d\n", rdma_name.c_str(), rdma_table[rdma_name].curr_prog_rqst);
				cv_wait_rqst.wait(lock);
			}
			//printf("%s: gotRqst curr_prog_rqst=%d progSzCPR=%d\n", rdma_name.c_str(),
			//	rdma_table[rdma_name].curr_prog_rqst,
			//	rdma_table[rdma_name].prog_table[rdma_table[rdma_name].curr_prog_rqst].prog_sz);

			/* If program request is not registered, return error. */
			if(!(rdma_table[rdma_name].prog_table.count(rdma_table[rdma_name].curr_prog_rqst))){
				return ERR_NO_GRP;
			}
			rdma_table[rdma_name].curr_rdma_state = busy;
			cv_wait_busy.notify_all();
			do_run = true;
			/* Check if program reqest for this rdma is null. */
			if(rdma_table[rdma_name].prog_table[rdma_table[rdma_name].curr_prog_rqst].prog){
				do_programming = false;
				if(rdma_table[rdma_name].curr_prog_rqst != rdma_table[rdma_name].curr_grp_id){
					do_programming = true;
				}
			}
			else{
				/* If program reqest is null, it is assumed that this rdma should do nothing dor this prog group.
				Thus, do_run will be false, and state will go directly to complete. */
				do_run = false;
			}
		} // release lock by scope exit

		if(do_run){
			/* Run prog reqest found. We will service the request to completion below.
			We must either program the rdma with the requested program, then run the program,
			or just run the program if the desired program is already in the rdma. */
			do {
				rdma_control.pread(&status, sizeof(status), 0);
				printf("%s: status 0x%x\n", rdma_name.c_str(), status);
				usleep(10000);
			} while ((status & 4) == 0); // wait for idle
			if(do_programming){
				{
					/* Access the semi-global table. */
					std::lock_guard<std::mutex> lock(m_rdma_table);
					prog = rdma_table[rdma_name].prog_table[rdma_table[rdma_name].curr_prog_rqst].prog;
					prog_sz = rdma_table[rdma_name].prog_table[rdma_table[rdma_name].curr_prog_rqst].prog_sz;
				}
				program_mem.pwrite(prog, prog_sz, 0);
				printf("%s: program written\n", rdma_name.c_str());
			}
			/* Run the program */
			data = CREG_0_RUN_PROGRAMS;
			rdma_control.pwrite(&data, sizeof(data), 0x20); // register at 0x20
			status = 1; // start
			rdma_control.pwrite(&status, sizeof(status), 0);
			printf("%s: program started\n", rdma_name.c_str());
			do {
				rdma_control.pread(&status,sizeof(status),0);
				//printf("%s: status 0x%x\n", rdma_name.c_str(), status);
				usleep(10000);
			} while ((status & 4) == 0); // wait for idle
		}
		printf("%s: completed\n", rdma_name.c_str());
		{
			/* Access the semi-global table. */
			std::lock_guard<std::mutex> lock(m_rdma_table);
			rdma_table[rdma_name].curr_rdma_state = complete;
			rdma_table[rdma_name].curr_grp_id = rdma_table[rdma_name].curr_prog_rqst;
			rdma_table[rdma_name].curr_prog_rqst = -1;
			cv_wait_complete.notify_all();
		}
	}
	return ERR_UNKOWN; // should never reach here, but make compiler happy
}

/**
 * @brief Runs RDMA program group passed as int argument. If using for load-on-demand (LOD)
 * the LOD group id can match the RDMA program group id (grp_id).
 * RDMAs must not be busy when called. Blocks until RDMAs complete.
 *
 * @param grp_id ID of program group to run.
 * @return error code
 */
int run_rdma_grp(int grp_id)
{
	/* Wait for total_registrations to be set. */
	{
		std::unique_lock<std::mutex> lock(m_prog_regis);
		while(total_registrations < 1){
			cv_wait_tot_regis.wait(lock);
		}
	}
	/* Wait for registration_count to equal total_registrations. */
	{
		std::unique_lock<std::mutex> lock(m_prog_regis);
		while(registration_count < total_registrations){
			cv_wait_regis_count.wait(lock);
		}
	}

	//printf("%s: funcBegin grp_id=%d\n", __FUNCTION__, grp_id);
	std::map<std::string, struct rdma_config>::iterator it;
	/* it->first is key, it->second is val */

	{
		std::unique_lock<std::mutex> lock(m_rdma_table);

		/* Make sure no RDMAs are currently busy. If so, return error. */
		for(it = rdma_table.begin(); it != rdma_table.end(); it++){
			if((it->second).curr_rdma_state == busy){
				return ERR_BUSY;
			}
		}

		/* Set the prog request for each RDMA. */
		bool found_grp = false;
		for(it = rdma_table.begin(); it != rdma_table.end(); it++){
			if((it->second).prog_table.count(grp_id)){
				(it->second).curr_prog_rqst = grp_id;
				found_grp = true;
			}
		}
		if(!found_grp){
			return ERR_NO_GRP;
		}
		cv_wait_rqst.notify_all();
		it = rdma_table.begin();
	}

	/* Wait for each RDMA in the group to trasistion to busy */
	//printf("%s: waitBusy grp_id=%d\n", __FUNCTION__, grp_id);
	while(1){
		{
			std::unique_lock<std::mutex> lock(m_rdma_table);
			if((it->second).prog_table.count(grp_id)){
				if((it->second).prog_table[grp_id].prog != NULL){
					while((it->second).curr_rdma_state != busy){
						cv_wait_busy.wait(lock);
					}
				}
			}
			it++;
			if(it == rdma_table.end()){
				it = rdma_table.begin();
				break;
			}
		}
	}

	/* Wait for each RDMA in the group to trasistion to complete */
	//printf("%s: waitComplete grp_id=%d\n", __FUNCTION__, grp_id);
	while(1){
		{
			std::unique_lock<std::mutex> lock(m_rdma_table);
			if((it->second).prog_table.count(grp_id)){
				while((it->second).curr_rdma_state != complete){
					cv_wait_complete.wait(lock);
				}
			}
			it++;
			if(it == rdma_table.end()){
				break;
			}
		}
	}

	//printf("%s: funcEnd grp_id=%d\n", __FUNCTION__, grp_id);
	return 0;
}

// debug helper
void print_rdma_table()
{
	std::map<std::string, struct rdma_config>::iterator it;
	std::lock_guard<std::mutex> lock(m_rdma_table);
	for(it = rdma_table.begin(); it != rdma_table.end(); it++){
		printf("PT: key=%s\n", (it->first).c_str());
		printf("curr_grp_id=%d curr_prog_rqst=%d curr_rdma_state=%d\n",
			(it->second).curr_grp_id, (it->second).curr_prog_rqst, (it->second).curr_rdma_state);
		// todo: use loop for prog_table
		//printf("prog_table[0].prog_sz=%d prog_table[1].prog_sz=%d\n",
		//	(it->second).prog_table[0].prog_sz, (it->second).prog_table[1].prog_sz);
	}
}
