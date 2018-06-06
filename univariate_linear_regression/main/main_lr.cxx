#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <string>
#include <cstdlib>


#include "zynq_ps_api.h"

//typedef struct {
//	char buf[256];
//} UserType;

extern "C" int main () {
	hls::stream<std::vector<TrainingExample>> *V_TrainingExample_Stream_arg_1_seq_i_0 = static_cast<hls::stream<std::vector<TrainingExample>>*>(sb_arg_1_seq_i_0);
	assert(V_TrainingExample_Stream_arg_1_seq_i_0 && "V_TrainingExample_Stream_arg_1_seq_i_0 is null");
	std::vector<TrainingExample> V_TrainingExample_arg_1_seq_i_0;
	std::string env_VSI_INSTALL = std::getenv("VSI_INSTALL");
	std::string path_training_example = env_VSI_INSTALL + "/target/common/hls_examples/univariate_linear_regression/main/data.txt";

	int M,N;
	std::ifstream f;
        f.open(path_training_example);
        if (f.fail())
        {       
                std::cout << "File not opened" << "\n";
                return 1;
        } 
        f >> M >> N;
	int feat=0;
        int targ = 0; 
	TrainingExample te(feat, targ);
	V_TrainingExample_arg_1_seq_i_0.push_back(te);
        //std::cout << "Num of training examples = " << M << ", Num of featurs = " << N << "\n";
        for (int i = 0; i < M; i++)
        {       
                f >> feat;
                f >> targ;
                //std::cout<<"feature="<<feat<<"\t target="<<targ<<"\n";
                TrainingExample te(feat, targ);
                V_TrainingExample_arg_1_seq_i_0.push_back(te);
        }
        f.close();

	hls::stream<std::vector<double>> *V_Theta_Stream_arg_2_seq_o_0 = static_cast<hls::stream<std::vector<double>>*>(sb_arg_2_seq_o_0);
	V_TrainingExample_Stream_arg_1_seq_i_0->write(V_TrainingExample_arg_1_seq_i_0);
	std::vector<double> V_Theta_arg_2_seq_o_0 = V_Theta_Stream_arg_2_seq_o_0->read();
	
	//for(int n=1; n< V_Theta_arg_2_seq_o_0.size(); n++) {
        //std::cout << V_Theta_arg_2_seq_o_0[n] << '\n';
    //}

}
