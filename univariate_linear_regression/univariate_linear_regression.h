#ifndef UNIVARIATE_H_
#define UNIVARIATE_H_

#include<iostream>
#include <vector>
#include <iostream>
#include <cmath>

class TrainingExample
{   
    private:
        int feature;
        int target;
    public:
        TrainingExample(int feat, int tar)
        {   
            feature = feat;
            target = tar;
        }
        int getFeature() { return feature; }
        void setFeature(int a){feature =a;}
        int getTarget() { return target; }
};

class Hypothesis
{
    private:
                std::vector<double> theta;
                std::vector<TrainingExample> ts;
                unsigned mExamples, nFeatures;
        public:
        double H(double theta_0, double theta_1, int feat)
                {
                        return (theta_0 + (theta_1 * feat ));;
                }
        double J()
        {
            double sum = 0.0;
            for (unsigned i = 1; i < mExamples; i++)
            {
                double diff = H(theta[0], theta[1], ts[i].getFeature()) - ts[i].getTarget();
                sum += diff*diff;
            }
            return sum / (2.0*(mExamples-1));
        }

        Hypothesis(std::vector<TrainingExample> examples,std::vector<double> THETA_init)
        {
		copy(THETA_init.begin(), THETA_init.end(), back_inserter(theta));
                ts=examples;
                mExamples = examples.size();
        }

        std::vector<double> gradientDescent()
        {
            const double alpha = 0.0000001;
            const double eps   = 0.00001;
            bool converge = false;
            int debug = 0;
            double HH;
            double sum_thetha_0, sum_thetha_1;
            std::vector<double> newTheta;
            while (!converge)
            {
                newTheta = theta;
                sum_thetha_0=0;
                sum_thetha_1=0;
                //cout << "J(theta) = " << J() << endl << endl;

                for (unsigned i = 1; i < mExamples; i++)
                {
                    HH = H(newTheta[0], newTheta[1], ts[i].getFeature());
                    //cout<< "Using example" << i  << "\t Htetha(x)=" << HH << endl;
                    sum_thetha_0 += HH - ts[i].getTarget();
                    sum_thetha_1 += (HH - ts[i].getTarget()) * ts[i].getFeature();
                }
                newTheta[0]=newTheta[0] - alpha*(sum_thetha_0/(mExamples-1));
                newTheta[1]=newTheta[1] - alpha*(sum_thetha_1/(mExamples-1));
                converge = true;
                converge = converge && (fabs(theta[0]-newTheta[0])<eps) && (fabs(theta[1]-newTheta[1])<eps);
                theta=newTheta;
             }
             return theta;
        }

};
#endif
