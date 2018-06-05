#include <iostream>
#include <fstream>
#include "univariate_linear_regression.h"
#include <hls_stream.h>
#include <unistd.h>

void linear_regression(hls::stream<std::vector<TrainingExample> > &ind, hls::stream<std::vector<double> > &Theta)
{
        std::vector<TrainingExample> ts = ind.read();
        //std::cout<< "size of training list in Algorithm:"<<ts.size()-1<<"\n";
	//std::cout<< "List of Training Examples in Algorithm:\n";
        //for (unsigned i = 1; i < ts.size(); i++)
        //	std::cout << "Example " << i << ": " << ts[i].getFeature() << "\t" << ts[i].getTarget()<<"\n";

        std::vector<double> THETA;
        THETA.push_back(0.0);
        THETA.push_back(0.0);
        Hypothesis hyp(ts,THETA);
        //std::cout<<"with initial thetas to 0, the cost function J()="<<hyp.J()<<"\n";
        THETA= hyp.gradientDescent();
	THETA.insert( THETA.begin(),200000); 
        Theta.write(THETA);
        //std::cout << "The last cost function J(theta) = " << hyp.J() << "\n \n";
	std::cout << "H_theta(x)="<<THETA[1]<<" + "<<THETA[2]<<"x \n";
	
	sleep(2);
}
