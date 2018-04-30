#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
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

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    BENCHMARK_DECLARE(deserialize)
    BENCHMARK_DECLARE(detect)
    try
    {
        // This example takes in a shape model file and then a list of images to
        // process.  We will take these filenames in as command line arguments.
        // Dlib comes with example images in the examples/faces folder so give
        // those as arguments to this program.
        if (argc == 1)
        {
            cout << "Call this program like this:" << endl;
            cout << "./face_landmark_detection_ex shape_predictor_68_face_landmarks.dat faces/*.jpg" << endl;
            cout << "\nYou can get the shape_predictor_68_face_landmarks.dat file from:\n";
            cout << "http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
            return 0;
        }

        // We need a face detector.  We will use this to get bounding boxes for
        // each face in an image.
        frontal_face_detector detector = get_frontal_face_detector();
        // And we also need a shape_predictor.  This is the tool that will predict face
        // landmark positions given an image and face bounding box.  Here we are just
        // loading the model from the shape_predictor_68_face_landmarks.dat file you gave
        // as a command line argument.
        shape_predictor sp;
        BENCHMARK_START
        deserialize(argv[1]) >> sp;
		BENCHMARK_MEASURE(deserialize)
		BENCHMARK_SUMMARY(deserialize)


        // Loop over all the images provided on the command line.
        for (int i = 2; i < argc; ++i)
        {
            BENCHMARK_START
            cout << "processing image " << argv[i] << endl;
            array2d<rgb_pixel> img;
            load_image(img, argv[i]);
            // Make the image larger so we can detect small faces.
            pyramid_up(img);

            // Now tell the face detector to give us a list of bounding boxes
            // around all the faces in the image.
            std::vector<rectangle> dets = detector(img);
            cout << "Number of faces detected: " << dets.size() << endl;

            // Now we will go ask the shape_predictor to tell us the pose of
            // each face we detected.
            std::vector<full_object_detection> shapes;
            for (unsigned long j = 0; j < dets.size(); ++j)
            {
                full_object_detection shape = sp(img, dets[j]);
                cout << "number of parts: "<< shape.num_parts() << endl;
                cout << "pixel position of first part:  " << shape.part(0) << endl;
                cout << "pixel position of second part: " << shape.part(1) << endl;
                // You get the idea, you can get all the face part locations if
                // you want them.  Here we just store them in shapes so we can
                // put them on the screen.
                shapes.push_back(shape);
            }

            // We can also extract copies of each face that are cropped, rotated upright,
            // and scaled to a standard size as shown here:
            dlib::array<array2d<rgb_pixel> > face_chips;
            extract_image_chips(img, get_face_chip_details(shapes), face_chips);
	        BENCHMARK_MEASURE(detect)
            BENCHMARK_SUMMARY(detect)

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
