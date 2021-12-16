
#include <thread>
#include <chrono>
#include <vector>

#include "ai_send_driver.h"
#include "imgprocessing.h"

// Use img as globalto share betwean read/write initial structure
img_struct *img;
// Image have ben read
bool rd_done = false;

/**
 * Reads an image file into the internal structure.
 *   *Supported tiff RGB 512x512 image format.
 *
 * @param img_path Path to the file.
 *
 * @returns Pointer into image structure;
 */
img_struct * read_img ( std::string img_path ) {
    img_struct *input_img = new img_struct();

    std::ifstream file(img_path, std::ios::binary | std::ios::ate);
    if ( !file ) {
        std::cerr  << "Input file reading fail.\n"
            << "Target file: " << img_path << std::endl;
        exit(255);
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (sizeof(img_struct) != size) {
        std::cerr  << "The expected structure does not match with file size.\n"
                << "Check if your file is tiff RGB 512x512.\n"
                << "Target file: " << img_path << std::endl;
        file.close();
        exit(255);
    }

    if (file.read((char *)input_img, sizeof(img_struct))) {
        file.close();
        return input_img;
    }

    file.close();
    std::cerr  << "Fail.";
    exit(255);
    return nullptr;
}

/**
 * Writes the internal structure into the image file.
 *   *Supported tiff RGB 512x512 image format.
 *
 * @param img_path Path to output the file.
 * @param img Pointer into the image structure.
 */
void write_img ( std::string img_path, img_struct * img ) {
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

/**
 * Reads an image file and send it to all channels through mm_to_ai IP.
 *
 * @param mem_out interface connected to the mm_to_ai IP.
 *
 */
void img_to_streams (vsi::device<int> &mem_out) {

    img = read_img ( INPUT_IMAGE_PATH  );
    rd_done = true;
    mem_stream_init(mem_out, N_STREAMS);
	const int img_size = sizeof(img->raw_img);
    // Send the same imgage to the all streams
	int *shared_buf_pois[N_STREAMS];
	for (int i = 0 ; i < N_STREAMS; i++) {
		shared_buf_pois[i] = (int *) img->raw_img;
	}

	int channel = 0;
	do {
        int remain_bytes = img_size - ((long)shared_buf_pois[channel] - (long)img->raw_img);
        if (remain_bytes) {
            int send_bytes = channel_write(mem_out, shared_buf_pois[channel], remain_bytes, channel);
            shared_buf_pois[channel] = shared_buf_pois[channel] + send_bytes/sizeof(int);
            // Should not be les then 0
            if ( (remain_bytes - send_bytes) <= 0 ) {
                printf("Image # %d sent \n", channel);
            }
        }
        channel = ((channel + 1) >= N_STREAMS) ? 0 : channel + 1;

	} while(1);
}

/**
 * Reads streams of the ai_to_mm IP and write content into image files.
 *
 * @param mem_out interface connected to the mm_to_ai IP.
 *
 */
void streams_to_img (vsi::device<int> &mem_input) {
    // Wait when image will be read
    while( !rd_done ) { };

    // Images cames from the streams
    std::vector <img_struct *> process_img_vector;
    // Output image names
    std::vector <std::string *> process_img_name;
    // Remain bytes of images
    std::vector <int> img_remain_bytes;
    // Tail pointer of images
    std::vector <int *> img_poi;
    // Initialization of output images
	for (int i = 0 ; i < N_STREAMS; i++) {
        img_struct *channel_image = new img_struct;
        // Copy geometry from the input pcture
        std::memcpy((void* ) channel_image->head, (const void* ) img->head, sizeof (img->head));
        std::memcpy((void* ) channel_image->tail, (const void* ) img->tail, sizeof (img->tail));
        process_img_vector.push_back(channel_image);
        // Build output picture name
        std::string  img_name = (std::string)"processed_img_" + std::to_string(i) + ".tiff";
        process_img_name.push_back(new std::string(img_name) );
        // Initialize pointers and remain pixels to compleat current image
        img_remain_bytes.push_back(sizeof(channel_image->raw_img) );
        img_poi.push_back( (int *) (channel_image->raw_img) );
	}

	// program the registers of stream to mm IP
    mem_stream_init(mem_input, N_STREAMS);
	int channel = 0;
	while(1) {
        if(img_remain_bytes[channel]) {
            // Reading data if data are available.
            int read_bytes = channel_read(mem_input,
                                          img_poi[channel],
                                          img_remain_bytes[channel],
                                          channel);
            img_poi[channel] += read_bytes/sizeof(int);
            img_remain_bytes[channel] -= read_bytes;

            // Flush the image into the file if data completed.
            if( img_remain_bytes[channel] == 0) {
                printf("Channel %d read done! \n", channel);
                write_img (*process_img_name[channel],
                           process_img_vector[channel]);
            }
        }

        // Move to the next channel
        channel++;
        if (channel >= N_STREAMS) {
            // Check all image if any of them not compleat need some data
            bool have_unprocess = false;
            for(auto remain_bytes: img_remain_bytes) {
                if(remain_bytes) {
                    have_unprocess = true;
                    break;
                }
            }

            if (!have_unprocess) {
                printf("Processing is done for all channels.\n");
                exit(0);
            }

            channel = 0 ;
        }

	} // stall forever
}
