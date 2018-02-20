#ifndef DOUBLE_BUFFER_H
#define DOUBLE_BUFFER_H
#ifndef __VSI_HLS_SYN__

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <mutex>
template <typename T>
class ProducerConsumerDoubleBuffer {
public:
	bool m_wc ;
	ProducerConsumerDoubleBuffer() {
		m_write_busy = false;
		m_read_idx = m_write_idx = 0;
		m_wc = false;
	}
	
	~ProducerConsumerDoubleBuffer() { }
	
	// The writer thread using this class must call
	// start_writing() at the start of its iteration
	// before doing anything else to get the pointer
	// to the current write buffer.
	T * start_writing(void) {
		mw_mutex.lock();
		std::lock_guard<std::mutex> lock(m_mutex);
		m_write_busy = true;
		m_write_idx = 1 - m_read_idx;
		return &m_buf[m_write_idx];
	}
	// The writer thread must call end_writing()
	// as the last thing it does
	// to release the write busy flag.
	void end_writing(void) {
		std::lock_guard<std::mutex> lock(m_mutex);
		
		m_write_busy = false;
		m_wc = true;
		mw_mutex.unlock();
	}
	
	// The reader thread must call start_reading()
	// at the start of its iteration to get the pointer
	// to the current read buffer.
	// If the write thread is not active at this time,
	// the read buffer pointer will be set to the 
	// (previous) write buffer - so the reader gets the latest data.
	// If the write buffer is busy, the read pointer is not changed.
	// In this case the read buffer may contain stale data,
	// it is up to the user to deal with this case.
	T * start_reading(void) {
		mr_mutex.lock();
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_write_busy) {
			m_read_idx = m_write_idx;
		}
		
		return &m_buf[m_read_idx];
	}
	// The reader thread must call end_reading()
	// at the end of its iteration.
	void end_reading(void) {
		std::lock_guard<std::mutex> lock(m_mutex);
		
		m_read_idx = m_write_idx;
		mr_mutex.unlock();
	}
	
private:
	T m_buf[2];
	bool m_write_busy;
	unsigned int m_read_idx, m_write_idx;
	std::mutex m_mutex;
	std::mutex mw_mutex;
	std::mutex mr_mutex;
};

#endif
#endif
