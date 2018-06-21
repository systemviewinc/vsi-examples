#include "regression_tests.h"

void stream_w_last_sum(hls::stream<test_last_type<DATA_WIDTH> > &in_stream,
		 hls::stream<test_last_type<DATA_WIDTH> > &out_stream)
{
	test_last_type<DATA_WIDTH> in;
	test_last_type<DATA_WIDTH> out;
	int count = 0;

	do {
#pragma HLS PIPELINE II=1
		in = in_stream.read();
		count++;
		if(in.last == 1)
			out.data = (ap_uint<DATA_WIDTH>)count;
		else
			out.data = in.data*2;
		out.last = in.last;
		out_stream.write(out);
	} while (in.last == 0);
}

void stream_w_last_user_sum(hls::stream<test_user_last_type<DATA_WIDTH> > &in_stream,
		 hls::stream<test_user_last_type<DATA_WIDTH> > &out_stream)
{
	test_user_last_type<DATA_WIDTH> in;
	test_user_last_type<DATA_WIDTH> out;
	int count = 0;

	do {
#pragma HLS PIPELINE II=1
		in = in_stream.read();
		count++;
		if(in.last == 1)
			out.data = (ap_uint<DATA_WIDTH>)count;
		else
			out.data = in.data*in.user;
		out.keep = in.keep;
		out.last = in.last;
		out.id   = in.id;
		out.user = in.user*2;
		out_stream.write(out);
	} while (in.last == 0);
}


#ifndef __VSI_HLS_SYN__

int compare_arrays(int array_one[], int array_two[], int size) {
    int ret = 0;
    printf("Index\t\t\tInput\t\tOutput\n");
    for (int i = 0 ; i < size; i++) {
		printf("%i\t\t0x%08x\t\t0x%08x\t", i, array_one[i], array_two[i]);
        printf((array_one[i] != array_two[i]) ? "X\n" : "\n");
        ret += (array_one[i] != array_two[i]);
    }

    return ret;
}

void fill_arrays(int array_one[], int size) {
    for (int i = 0 ; i < size; i++) {
		array_one[i] = rand();
    }
}

void random_data_send(hls::stream<int> &out_stream, hls::stream<int> &in_stream,
                    hls::stream<test_last_type<DATA_WIDTH> > &out_stream_last, hls::stream<test_last_type<DATA_WIDTH> > &in_stream_last,
					hls::stream<test_user_last_type<DATA_WIDTH> > &out_stream_user, hls::stream<test_user_last_type<DATA_WIDTH> > &in_stream_user,
                    vsi::device &out_sort_mem, vsi::device &in_sort_mem, vsi::device &bram_memory,
                    vsi::device &GPIO_0, vsi::device &GPIO_1)
{
    int out_arr[1024];
    int in_arr[1024];
    int sw_sort[1024];

	test_last_type<DATA_WIDTH> in_last;
	test_last_type<DATA_WIDTH> out_last;
	test_user_last_type<DATA_WIDTH> in;
	test_user_last_type<DATA_WIDTH> out;

    char* test;
	char* size;
	char* num_tests;
    int gpio_write = 0xDEADBEEF;
    int gpio_read  = 0x0F0F0F0F;
    int ret, i, t, test_val, size_val, number_of_tests;

	srand(time(NULL));
    test = getenv("TEST");
	num_tests = getenv("NUM_TESTS");
	size = getenv("SIZE");

	//check to make sure we have a  file and size
	if (test!=NULL) {
        test_val = (int)strtol(test, NULL, 10);
    }
    else{
        printf("Set TEST env variable\n");
        exit(0);
    }
	//check to make sure we have a  file and size
	if (num_tests!=NULL) {
		number_of_tests = (int)strtol(num_tests, NULL, 10);
	}
	else{
		number_of_tests = 1;
	}
	printf("Running test type %d!\n", test_val);
	printf("Running %d number of tests!\n", number_of_tests);



    switch(test_val) {

        case 0:
			//Streaming with last testvar
			//check to make sure we have a  file and size
			if (size!=NULL) {
				size_val = (int)strtol(size, NULL, 10);
			}
			else{
				printf("Set SIZE env variable\n");
				exit(1);
			}

			for(t = 0; t < number_of_tests; t++) {
				//write out_stream_last
				printf("Write streaming with last size 0x%x\n", size_val);
				for(i = 0; i < size_val; i++){
					out_last.data = (ap_uint<DATA_WIDTH>)out_arr[i];
					if(i == size_val -1){
						out_last.last = 1;
					} else {
						out_last.last = 0;
					}
					out_stream_last.write(out_last);
				}

				//read input gpio
				printf("Reading streaming with last size 0x%x\n", size_val);
				for(i = 0; i < size_val; i++){
					in_stream_last.read(in_last);
					in_arr[i] = in_last.data;
					if(i == size_val -1){
						//out.last = 1;
					} else {
						//out.last = 0;
					}
				}
				sleep(10);

				ret = compare_arrays(out_arr, in_arr, size_val);
				if(ret){
					printf("ERROR STREAMING WITH LAST test failed! \n");
					exit(1);
				}
			}
			break;

		case 1:
			//Streaming with user testvar
			//check to make sure we have a  file and size
			if (size!=NULL) {
				size_val = (int)strtol(size, NULL, 10);
			}
			else{
				printf("Set SIZE env variable\n");
				exit(1);
			}

			for(t = 0; t < number_of_tests; t++) {
				//write out_stream_last
				printf("Write streaming with last size 0x%x\n", size_val);


				for(i = 0; i < size_val; i++){

					out.data = (ap_uint<DATA_WIDTH>)out_arr[i];
					out.keep = -1;
					if(i == size_val -1){
						out.last = 1;
					} else {
						out.last = 0;
					}
					out.id   = 1;
					out_stream_user.write(out);
				}

				//read input gpio
				printf("Reading streaming with last size 0x%x\n", size_val);

				for(i = 0; i < size_val; i++){
					in_stream_user.read(in);

					in_arr[i] = in.data;

					if(i == size_val -1){
						//out.last = 1;
					} else {
						//out.last = 0;
					}
				}
				sleep(10);

				ret = compare_arrays(out_arr, in_arr, size_val);
				if(ret){
					printf("ERROR STREAMING WITH USER & LAST test failed! \n");
					exit(1);
				}
			}
			break;


        case 2:
            //HW STREAMING
			for(t = 0; t < number_of_tests; t++) {

				fill_arrays(out_arr, 1024);

	            printf("Write out stream\n");
	            out_stream.write(&out_arr,sizeof(out_arr));

	            printf("read in_stream\n");
				ret = 0;
				while(ret < 1024*4){
					ret += in_stream.read(&in_arr[ret/4],sizeof(in_arr));
					printf("read amount %d\n", ret);
				}


	            ret = compare_arrays(out_arr, in_arr, 1024);
	            if(ret){
	                printf("ERROR STREAMING test failed! \n");
	                exit(1);
	            }
			}
            break;

        case 3:
            //BRAM TEST
			for(t = 0; t < number_of_tests; t++) {

				fill_arrays(out_arr, 1024);

	            printf("Write BRAM\n");
	            bram_memory.pwrite(&out_arr,sizeof(in_arr),0);
	            printf("Read BRAM\n");
	            bram_memory.pread(&in_arr,sizeof(in_arr),0);

	            ret = compare_arrays(out_arr, in_arr, 1024);
	            if(ret){
	                printf("ERROR BRAM test failed! \n");
	                exit(1);
	            }
			}
            break;

        case 4:

            //SW sort
			for(t = 0; t < number_of_tests; t++) {

				fill_arrays(out_arr, 1024);

	            printf("Running software sort\n");
	            quick_sort(out_arr, sw_sort);

	            //HW sort

	            printf("Write HW sort\n");
	            out_sort_mem.pwrite(&out_arr,sizeof(out_arr),0);

	            printf("Poll HW sort\n");
	            in_sort_mem.poll(-1);

	            printf("Read HW sort\n");
	            in_sort_mem.pread(&in_arr,sizeof(in_arr),0);

	            ret = compare_arrays(sw_sort, in_arr, 1024);
	            if(ret){
	                printf("ERROR SORT test failed! \n");
	                exit(1);
	            }
			}
            break;

        case 5:

            //GPIO TEST
			for(t = 0; t < number_of_tests; t++) {

	            //write output gpio
				gpio_write = rand();
	            GPIO_0.pwrite(&gpio_write,sizeof(gpio_write),0);
	            printf("Wrote \t0x%08x to output GPIO\n", gpio_write);


	            //read input gpio
	            GPIO_1.pread(&gpio_read,sizeof(gpio_read),0);
	            printf("Read  \t0x%08x from input GPIO\n", gpio_read);
	            if(gpio_read != gpio_write){
	                printf("ERROR GPIO test failed! \n");
	                exit(1);
	            }
			}
            break;



        default:
            printf("Select a case between 0 - 5 \n");

    }

	printf("Tests completed sucessfully \n");
    exit(0);
}

void run_sort(vsi::device &out_sort_mem, vsi::device &in_sort_mem)
{
    int write_arr[1024];
    int read_arr[1024];
    int sw_sort[1024];

    char* test;
	char* size;
	char* num_tests;
    int gpio_write = 0xDEADBEEF;
    int gpio_read  = 0x0F0F0F0F;
    int ret, t, number_of_tests;

	srand(time(NULL));
	num_tests = getenv("NUM_TESTS");

	//check to make sure we have a  file and size
	if (num_tests!=NULL) {
		number_of_tests = (int)strtol(num_tests, NULL, 10);
	}
	else{
		number_of_tests = 1;
	}
	printf("Running %d number of sorts!\n", number_of_tests);

    //SW sort
	for(t = 0; t < number_of_tests; t++) {

		fill_arrays(write_arr, 1024);

        //HW sort

        printf("Write HW sort\n");
        out_sort_mem.pwrite(&write_arr,sizeof(write_arr),0);

        printf("Poll HW sort\n");
        in_sort_mem.poll(-1);

        printf("Read HW sort\n");
        in_sort_mem.pread(&read_arr,sizeof(read_arr),0);

		printf("Running software sort\n");
        quick_sort(write_arr, sw_sort);

        ret = compare_arrays(read_arr, sw_sort, 1024);
        if(ret){
            printf("ERROR SORT test failed! \n");
            exit(1);
        }
	}


	printf("Sort completed sucessfully \n");
    exit(0);
}

#endif
