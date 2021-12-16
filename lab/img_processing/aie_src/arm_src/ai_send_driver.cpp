#include "ai_send_driver.h"
#include <thread>
#include <chrono>
#include <vector>

#define N_STREAMS 1

int shared_buf[] = { 0x01020304, 0x05060709, 0x0AB0C0D0, 0x0AB0C0D0, 0x0AB0C0D0, 0x0AB0C0D0 };

typedef struct rgb_pixel {
    char red;
    char green;
    char blue;
};

#define HEIGHT 512
#define WIDTH 512

// tiff format
typedef struct img_struct {
    char head[8];
    rgb_pixel raw_img[HEIGHT*WIDTH];
    char tail[132];
};
img_struct *img;
bool rd_done = false;

// Only 512x512 supported
img_struct * read_img ( std::string img_path ) {
    img_struct *input_img = new img_struct();

    std::ifstream file(img_path, std::ios::binary | std::ios::ate);
    if ( !file ) {
        std::cerr  << "Input file reading fail.";
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (sizeof(img_struct) != size) {
        std::cerr  << "Aspectet structure not match with a file size.";
    }

    if (file.read((char *)input_img, sizeof(img_struct))) {
        file.close();
        return input_img;
    }

    file.close();
    std::cerr  << "Fail.";
    return nullptr;
}

// Only 512x512 supported
void write_img ( std::string img_path ,img_struct * img ) {
    img_struct *input_img = new img_struct();

    std::ofstream file(img_path, std::ios::binary | std::ios::out);
    if ( !file ) {
        std::cerr  << "Cannot open file : " << img_path << "\n";
        return;
    }

    if (file.write((char *)img, sizeof(img_struct))) {
        std::cout  << "Output file: " << img_path << "\n";
    } else {
        std::cerr  << "File writing failed: " << img_path << "\n";
    }

    file.close();

}

void aximm_to_streams_init (
    vsi::device<int> &mem,
    unsigned int data_offset,
    unsigned int channel_number ) {

	for (int i = 0 ; i < channel_number*4; i++) {
#pragma HLS pipeline II=1
		int addr = i*4;
		// val = 0x20000 + (i << 12) ;
		int val = data_offset + (i << 12) ;
		int reg = (i&3);
		switch (reg) {
		case 1:
			// high range
			val |= 0xfff;

		case 0:
			// low range
			blocked_write(mem, &val, sizeof(val), addr);
			break;

		default:
			// level & full registers skip
			break;
		}
	}

}

void streams_to_aximm_init (
    vsi::device<int> &mem,
    unsigned int base_adr,
    unsigned int channel_number)
{
    aximm_to_streams_init(mem, base_adr, channel_number);
};

void get_channel_level (
    vsi::device<int> &mem,
    int *efull,
    int *level,
    unsigned int channel_number)
{
    // int a[2];
    // blocked_read(mem_out, a, sizeof(a), ((4*channel_number) + 2)*4);
    blocked_read(mem, efull, sizeof(int), ((4*channel_number) + 2)*4);
    blocked_read(mem, level, sizeof(int), ((4*channel_number) + 3)*4);
};





// Hardware requires it to be a struct
void img_to_streams (vsi::device<int> &mem_out) {
    //Test
//std::ofstream file("raw", std::ios::binary | std::ios::out);

//

    /*
   write_img
    */
    img = read_img ( "lena512color.tiff" );
    rd_done = true;

	int len = sizeof(shared_buf)/sizeof(int);
    unsigned int val;
    unsigned int data_offset = 0x20000;

    aximm_to_streams_init(mem_out, data_offset, N_STREAMS);

	int i = 0;
	const int img_size = sizeof(img->raw_img);
    // exit(0);
	int *shared_buf_pois[N_STREAMS];
	for (int i = 0 ; i < N_STREAMS; i++) {
		shared_buf_pois[i] = (int *) img->raw_img;
	}

	const int pack_size = 256;
    int efull, level;
	do {
		int addr = data_offset + (i << 14);
        get_channel_level(mem_out, &efull, &level, i);
        // printf("img_to_streams: efull = %d, level = %d\n", efull, level);
        // while (1){};

		if (level < pack_size) {
		 	// send data to the stream
			int remain_bytes = img_size - ((long)shared_buf_pois[i] - (long)img->raw_img);
            if ( !remain_bytes ) {
                std::cout << "Image sent\n";
// file.close();
                while(1) {};
            }
			int send_bytes = remain_bytes > pack_size ? 4*pack_size : remain_bytes;
		 	// Sending bytes to a stream
            // printf("Sending %d bytes\n", send_bytes);
			blocked_write(mem_out, shared_buf_pois[i], send_bytes, addr);
// Test to see wat do I send to AI
//  file.write((char *)shared_buf_pois[i],  (send_bytes) );

            //
			shared_buf_pois[i] = remain_bytes > pack_size ?
                shared_buf_pois[i] + send_bytes/4 :
                shared_buf;
		}

		i++;
		if (i >= N_STREAMS) {
			i = 0 ;
		}
//  std::this_thread::sleep_for(std::chrono::milliseconds(100));

	} while(1);
}

void streams_to_img (vsi::device<int> &mem_input) {



    // Wait when image will be read
    while( !rd_done ) { };

    // img_struct *process_img = new img_struct;

    // std::memcpy((void* ) process_img->head, (const void* ) img->head, sizeof (img->head));
    // std::memcpy((void* ) process_img->tail, (const void* ) img->tail, sizeof (img->tail));
    // for(int x = 0; x < HEIGHT; x++){
    //     for(int y = 0; y < WIDTH; y++){
    //         process_img->raw_img[x][y].red = (char) x;
    //         process_img->raw_img[x][y].green = (char) y;
    //         process_img->raw_img[x][y].blue = (char) x+y;
    //     }
    // }

    // write_img ("lena512proc.tiff", process_img);
//  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

// exit(0);
//  while(1); // stall forever

  unsigned int val;
    // Use this buffer as a drain for any remain data which could stuck in fifo
    int drain_buffer[64];

	ofstream logfiles[N_STREAMS];
	char file_name[64];

    std::vector <img_struct *> process_img_vector;
    std::vector <std::string *> process_img_name;
    std::vector <int> img_remain_bytes;
    std::vector <int *> img_poi;

    // std::memcpy((void* ) process_img->head, (const void* ) img->head, sizeof (img->head));
    // std::memcpy((void* ) process_img->tail, (const void* ) img->tail, sizeof (img->tail));

	for (int i = 0 ; i < N_STREAMS; i++) {
        img_struct *channel_image = new img_struct;
        std::memcpy((void* ) channel_image->head, (const void* ) img->head, sizeof (img->head));
        std::memcpy((void* ) channel_image->tail, (const void* ) img->tail, sizeof (img->tail));
        process_img_vector.push_back(channel_image);
        std::string  img_name = (std::string)"processed_img_" + std::to_string(i) + ".tiff";
        process_img_name.push_back(new std::string(img_name) );
        img_remain_bytes.push_back(sizeof(channel_image->raw_img) );
        img_poi.push_back( (int *) (channel_image->raw_img) );
	}

    unsigned int data_offset = 0x20000;
	// program the registers of stream to mm IP
    streams_to_aximm_init(mem_input, data_offset, N_STREAMS);
		unsigned int efull, a[2];
		int level;

//  while (1){};
	int i = 0;
    int int_level;
    int *rdpoi;
    int total_reads = 0;
	do {
        get_channel_level(mem_input, &efull, &level, 0);
        //
		if ( ((efull & 1) == 0) && img_remain_bytes[i] ) {
            // Number of integers in fifo
            // ints_in_fifo = level * (sizeof(int)/sizeof(char));
			/* empty this stream into a buffer */
			if(level > 4) {
                int buf_room = img_remain_bytes[i];
                rdpoi = img_poi[i];
				/* align the 16 bytes */
                int read_bytes = (4*level > buf_room ? buf_room : 4*level);
				read_bytes &= ~(16-1);
                img_poi[i] += read_bytes/sizeof(int);
				blocked_read(
                    mem_input,
                    rdpoi,
                    read_bytes,
                    data_offset + (i << 14));
                total_reads += read_bytes;
                // printf( " total_reads: %d", total_reads);
                img_remain_bytes[i] -= read_bytes;
//                 printf( " remain_bytes: %d", img_remain_bytes[i]);
//                 printf( " rdpoi: 0x%08X", rdpoi);
//                 printf( " img_poi: 0x%08X\n",  img_poi[i]);
//                 printf( "    diff: %d\n",   ( ( (long) img_poi[i] - (long ) ppp )) );

			}

            if( img_remain_bytes[i] == 0) {
                std::cout << "Read done!!\n";
                write_img (* process_img_name[i], process_img_vector[i]);
            }
		}

		i++;

		if (i >= N_STREAMS) {
            // Check all image if any need some data
            bool have_unprocess = false;
            for(auto remain_bytes: img_remain_bytes) {
                if(remain_bytes) {
                    have_unprocess = true;
                    break;
                }
            }

            if (!have_unprocess) {
                std::cout << "Processing done for all chanels.";
                std::cout << std::flush;

                exit(0);
            }

            i = 0 ;
        }

	} while(1); // stall forever
}

void pull_remain(vsi::device<int> &mem_input, int stream_number) {

}

// Print buffer to file
void printbuf(ofstream *logfile, int *buf, int len) {

  while ( len-- ) {
		*logfile << std::setfill('0') << std::setw(8) << std::hex << *buf << "\n";
		buf++;
	}
	logfile->flush();
}

void blocked_write (vsi::device<int> &mem_out, int* buf, int size, int addr) {
	mem_out.pwrite(buf, size, addr);
}

void blocked_read (vsi::device<int> &mem_out, int* buf, int size, int addr) {
	mem_out.pread(buf, size, addr);
}

// Computation buffer hash
void hash_compute(int *buf, int size) {
  int hash = 0;
  for(int i = 0; i < size; ++i) {
    hash ^= buf[i];
  }
  printf("Hash values of buffer :");
  printf("    %d\n", hash);
  printf("    0x%X\n", hash);
}
