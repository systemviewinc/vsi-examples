#include "ai_send_driver.h"

/**
 * Initialization of streams to memory and memory to streams IP's.
 *
 * @param mem interface connected to target IP.
 * @param channel_number The number of channels.
 */
void mem_stream_init (
    vsi::device &mem,
    unsigned int channel_number ) {

    int low_range;
    int hight_range;
    for (int channel = 0 ; channel < channel_number; channel++) {
        // Set low & hight memory ranges for current channel
        low_range = DATA_BASE_OFFSET + (channel * CHANNEL_SPAN);
        hight_range = low_range + (CHANNEL_SPAN - 1);
        mem.pwrite(&low_range, sizeof(low_range), CH_LOW_RANGE_REG(channel));
        mem.pwrite(&hight_range, sizeof(hight_range), CH_HIGHT_RANGE_REG(channel));
    }
}

/**
 * Get channel level and fifo fullness
 *
 * @param mem interface connected to target IP.
 * @param empty_full Fifo fullness. 0bit - empty, 1bit - full.
 * @param level Fifo fullness level in bytes.
 * @param channelThe channel number.
 */
void get_channel_level (
    vsi::device &mem,
    unsigned int *empty_full,
    unsigned int *level,
    unsigned int channel)
{
    mem.pread((int *)empty_full, sizeof(int), CH_EFULL_REG(channel));
    mem.pread((int *)level, sizeof(int), CH_LEVEL_REG(channel));
};

/**
 * Write buffer into the channel.
 * 
 * @param mem interface connected to target IP.
 * @param buf Pointer to the buffer.
 * @param n Number of bytes.
 * @param channel_number The channel number.
 * 
 * @returns a number of bytes written.
 */
int channel_write (vsi::device &mem, int* buf, int n, int channel) {
    // Check if number of bytese set correct
    if (n < 0) {
        return 0;
    }

    const int pack_size = 256; // Require room in bytes to initialize send
    unsigned int efull, level;
    get_channel_level(mem, &efull, &level, channel);

    int remain_bytes = n;
    int write_bytes = 0;
    if (level < pack_size)
    {
        write_bytes = n > pack_size ? 4*pack_size : n;
        /* align the 16 bytes */
        write_bytes &= ~(16-1);
        // Sending bytes to a stream
        mem.pwrite(buf, write_bytes, CH_DATA_ADDR(channel));
    }

    return write_bytes;
}

/**
 * Read from channel stream into the buffer.
 * 
 * @param mem interface connected to target IP.
 * @param buf Pointer to the buffer.
 * @param n Number of bytes.
 * @param channel_number The channel number.
 * 
 * @returns number of read bytes.
 */
int channel_read (vsi::device &mem, int* buf, int n, int channel) {
    // Check if number of bytese set correct
    if (n < 0) {
        return 0;
    }
    int read_bytes = 0;
    unsigned int empty_full;
    unsigned int level; // integers in fifo
    get_channel_level(mem, &empty_full, &level, channel);
    if ( ((empty_full & 1) == 0) && (level > 4) ) {
        // Reading available data if request more than FIFO contains.
        read_bytes = 4*level > n ? n : 4*level;
        /* align the 16 bytes */
        read_bytes &= ~(16-1);
        mem.pread(buf, read_bytes, CH_DATA_ADDR(channel));
    }

    return read_bytes;
}

void pull_remain(vsi::device &mem, int stream_numbers) {
    //  TODO: add pulling of any data from the channel
}


