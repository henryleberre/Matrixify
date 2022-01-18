mkdir -p output
mkdir -p temp
mkdir -p temp/input_frames
mkdir -p temp/output_frames
mkdir -p temp/input_characters

clang++ -I/usr/include/opencv4/ -L/usr/lib -lopencv_highgui -lopencv_dnn -lopencv_ml -lopencv_videoio -lopencv_video -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_photo -lopencv_imgproc -lopencv_core -I/usr/include/freetype2 -lfreetype -mavx -std=c++17 matrixify.cpp SimplexNoise/src/SimplexNoise.cpp -o matrixify

./matrixify
