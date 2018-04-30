These demos use dlib. THe library is bundled with VSI. 
The face_detection.cxx only finds the faces in an image. The smaller the size, the less time it takes.
For an ARM processor, try to resize the input image to 96x96px to get reasonable performance.

Following command can be used to compile the executable:

X86:

g++ face_detection.cxx -o face_detection -I $VSI_INSTALL/target/X86/include -std=c++11 -L$VSI_INSTALL/target/X86/lib  -ldlib -lopenblas -static -pthread -DBENCHMARK
g++ face_landmark.cxx -o face_landmark -I $VSI_INSTALL/target/X86/include -std=c++11 -L$VSI_INSTALL/target/X86/lib  -ldlib -lopenblas -static -pthread -DBENCHMARK


ARM64:

aarch64-linux-gnu-g++ face_detections.cxx -o face_detection_arm64 -I $VSI_INSTALL/target/ARM64/include -std=c++11 -L$VSI_INSTALL/target/ARM64/lib  -ldlib -lopenblas -static -pthread -DBENCHMARK
aarch64-linux-gnu-g++ face_landmark.cxx -o face_landmark -I $VSI_INSTALL/target/ARM64/include -std=c++11 -L$VSI_INSTALL/target/ARM64/lib  -ldlib -lopenblas -static -pthread -DBENCHMARK

Run:
./face_detection faces/Tom_Cruise_avp_2014_4_96.jpg

Download the model for DNN example:

wget http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2
tar xf shape_predictor_68_face_landmarks.dat.bz2

./face_landmark shape_predictor_68_face_landmarks.dat faces/*.jpg 
