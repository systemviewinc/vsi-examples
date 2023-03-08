#include <adf.h>
#include <aie_api/aie.hpp>
#include <aie_api/aie_adf.hpp>
using namespace std;
void passthrough(
    input_window_float * in1, 
    output_window_float * outp) 
{
    float c1;
    for(int i = 0; i < 16*256; i++){
        c1 = window_readincr(in1);
        window_writeincr(outp, c1);
    }
}

void simple_add(
    input_window_float * in1, 
    output_window_float * outp) 
{
    float c1;
    for(int i = 0; i < 16*256; i++){
        c1 = window_readincr(in1);
        // float temp = static_cast<float>(i);
        // c1 = c1 + temp;
        window_writeincr(outp, (c1 + c1));
    }
}

void simple_mul(
    input_window_float * in1, 
    output_window_float * outp) 
{
    float c1;
    for(int i = 0; i < 16*256; i++){
        c1 = window_readincr(in1);
        window_writeincr(outp, (c1 * c1));
    }
}