#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_int.h>
#include <hls_video.h>
#include <stdint.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

#include "zynq_ps_api.h"

#define ROW_SIZE 320
#define COL_SIZE 320


extern "C" int main () {
	hls::stream<axis_uint8_type> *Input_Stream_arg_1_seq_i_0 = static_cast<hls::stream<axis_uint8_type>*>(sb_arg_1_seq_i_0);
	assert(Input_Stream_arg_1_seq_i_0 && "Input_Stream_arg_1_seq_i_0 is null");
	axis_uint8_type Input_arg_1_seq_i_0;
	std::string env_VSI_INSTALL = std::getenv("VSI_INSTALL");
        std::string path_pic = env_VSI_INSTALL + "/target/common/hls_examples/hls_video/main/Valve_original.csv";
	int row=0;
	int col=0;
	int data[ROW_SIZE][COL_SIZE];
	int modified_data[ROW_SIZE][COL_SIZE];
    	std::ifstream file(path_pic);
	if (file.fail())
          {       
                std::cout << "CSV file doesn't exist in the"<<path_pic<<" \n";
                return 1;
          }
        else
          {
        	std::cout << "CSV file is opened from: "<<path_pic<<"\n Please wait . . . \n";
          } 
	

    	for(row = 0; row < ROW_SIZE; row++)
    	  {	
	    
            std::string line;
            std::getline(file, line);
	    if ( !file.good() )
            	{
		   break;
		   printf("break1\n");
		}

            std::istringstream iss(line);	
	    std::string val;
            while (std::getline(iss, val, ','))
              {
        	std::stringstream convertor(val);
            	convertor >> data[row][col];
                col++;
                if (col>=COL_SIZE)
                col=0;
              }
	
	  }
	for(row = 0; row < ROW_SIZE; row++)
          {
		for (col = 0; col < COL_SIZE; col++)
            	  {
//			printf("data[%d][%d]= %d \n",row,col,data[row][col]);
			Input_arg_1_seq_i_0.data = (uint8_t) data[row][col];
                	*Input_Stream_arg_1_seq_i_0 << Input_arg_1_seq_i_0;
//			printf("Input_arg_1_seq_i_0 data is: %d\n",(int) Input_arg_1_seq_i_0.data);
	    	  }
	  }	

	sleep(2);

	hls::stream<axis_uint8_type> *Output_Stream_arg_2_seq_o_0 = static_cast<hls::stream<axis_uint8_type>*>(sb_arg_2_seq_o_0);
	for(row = 0; row < ROW_SIZE; row++)
          {
	   	for (col = 0; col < COL_SIZE; col++)
            	  {	
			axis_uint8_type Output_arg_2_seq_o_0 = Output_Stream_arg_2_seq_o_0->read();
//        		std::cout << "Output:[" <<row<< "]["<<col<<"]="<<Output_arg_2_seq_o_0.data << '\n';
			modified_data[row][col]= (int) Output_arg_2_seq_o_0.data;
	    	  }
	  } 
//	std::cout << "total output row: "<<row << '\t'<<"total output col: "<<col <<'\n';

	std::ofstream out("Image_with_applied_Sobel_filter.csv");
	for (auto& row1 : modified_data) {
  	   for (auto col1 : row1)
    		out << col1 <<',';
  	   out << '\n';
	}

	std::cout << "Sobel Algorithm is finished.\n";

}
