/* A simple kernel
 */
#include <adf.h>
#include "include.h"

// If compiled with chess_unroll_loop not commented out, aiecompiler needs --large-kernel-program flag.

__attribute__((noinline))
void xil_func(input_window_cint16 * inp, output_window_cint16 * out) {
    cint16 c1, c2, c3;
    for (unsigned i=0; i<NUM_SAMPLES; i++)
    {
        window_readincr(inp, c1);
        c2.real = c1.real+c1.imag;
        c2.imag = c1.real-c1.imag;
        c3.real = c2.real+c2.imag;
        c3.imag = c2.real-c2.imag;
        window_writeincr(out, c3);
    }
    for (int x = 0; x < 40; ++x)
    chess_unroll_loop(*)
    {
        for (unsigned i=0; i<NUM_SAMPLES; i++)
        chess_unroll_loop(*)
        {
            window_readincr(inp, c1);
            c2.real = c1.real+c1.imag;
            c2.imag = c1.real-c1.imag;
            c3.real = c2.real+c2.imag;
            c3.imag = c2.real-c2.imag;
            window_writeincr(out, c3);
        }
    }
    for (unsigned i=0; i<NUM_SAMPLES; i++)
    {
        window_readincr(inp, c1);
        c2.real = c1.real+c1.imag;
        c2.imag = c1.real-c1.imag;
        c3.real = c2.real+c2.imag;
        c3.imag = c2.real-c2.imag;
        window_writeincr(out, c3);
    }
}

