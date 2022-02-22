#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_io.h>
#include <iostream>


#ifdef BENCHMARK
#include <chrono>
#include <sys/time.h>
using namespace std;
using namespace std::chrono;

#define BENCHMARK_DECLARE(func_type) \
double func_type ## _dif, func_type ## _avg = 0;

#define BENCHMARK_START t_start = high_resolution_clock::now();
#define BENCHMARK_MEASURE(func_type) \
		t_end = high_resolution_clock::now(); \
		func_type ## _dif = duration_cast<milliseconds>( t_end - t_start ).count(); \
		func_type ## _avg = (func_type ## _dif + func_type ## _avg) / 2;

#define BENCHMARK_SUMMARY(func_type) \
		printf (#func_type " avg: %lf ms\n", func_type ## _avg);

high_resolution_clock::time_point t_start, t_end;

#else
#define BENCHMARK_DECLARE(a)
#define BENCHMARK_START
#define BENCHMARK_MEASURE(a)
#define BENCHMARK_SUMMARY(a)
#endif

using namespace dlib;
using namespace std;

BENCHMARK_DECLARE(detect)
// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        if (argc == 1)
        {
            cout << "Give some image files as arguments to this program." << endl;
            return 0;
        }

        frontal_face_detector detector = get_frontal_face_detector();

        // Loop over all the images provided on the command line.
        for (int i = 1; i < argc; ++i)
        {
            BENCHMARK_START
            cout << "processing image " << argv[i] << endl;
            array2d<unsigned char> img;
            load_image(img, argv[i]);
            // Make the image bigger by a factor of two.  This is useful since
            // the face detector looks for faces that are about 80 by 80 pixels
            // or larger.  Therefore, if you want to find faces that are smaller
            // than that then you need to upsample the image as we do here by
            // calling pyramid_up().  So this will allow it to detect faces that
            // are at least 40 by 40 pixels in size.  We could call pyramid_up()
            // again to find even smaller faces, but note that every time we
            // upsample the image we make the detector run slower since it must
            // process a larger image.
            pyramid_up(img);

            // Now tell the face detector to give us a list of bounding boxes
            // around all the faces it can find in the image.
            std::vector<rectangle> dets = detector(img);

            // Now we will go ask the shape_predictor to tell us the pose of
            // each face we detected.

            for (unsigned long j = 0; j < dets.size(); ++j)
            {
                std::cout << dets[j] << "\n";
            }
	        BENCHMARK_MEASURE(detect)
            BENCHMARK_SUMMARY(detect)

            cout << "Number of faces detected: " << dets.size() << endl;

            cout << "Hit enter to process the next image..." << endl;
            cin.get();
        }
    }
    catch (exception& e)
    {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}

// ----------------------------------------------------------------------------------------

