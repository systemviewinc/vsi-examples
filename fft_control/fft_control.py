import vsi_runtime
import numpy

# This function will send random numbers to the fft code
def send_wave_file (fft_out, fft_in, fft_control):
    print('Start ')
    buff_out = vsi_runtime.Buffer(8192)  # buffer will have 1K * 2 (re,im) floats
    while True:
        print ('Loop')        
        for i in range(0, 1023):
            fft_out.putFloat(numpy.random.uniform(1.0,2.0)) # real
            fft_out.putFloat(0.0)			    # imaginary
        # send it to the fft core    
        fft_out.write(buff_out)
        # wait for it to process and send it back
        buff_in = fft_in.read()
        for i in range(0, 1023):
            s = 'Output from FFT ' + i + ' real ' + buff_in.getFloat() + ' im ' + buff_in.getFloat()
            print(s)
