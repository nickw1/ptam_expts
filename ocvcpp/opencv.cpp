#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <emscripten.h>
#include <iostream>

using namespace std;

extern "C" {
    EMSCRIPTEN_KEEPALIVE void receiveData(uint8_t *ptr,/*size_t size, */ int width, int height);
    EMSCRIPTEN_KEEPALIVE void test1(int i);
}


int main(int argc, char *argv[]) {
    cout << "c++ main" << endl;
    return 0;
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE void receiveData(uint8_t *ptr,/*size_t size, */int width, int height) {
        cout << "receiveData()" << endl;
        cout << "Width " << width << " Height " << height << " Pointer: " << ((long)ptr) << endl; 
        auto cv_image = cv::Mat(width, height, CV_8UC1, ptr);
        cout << "Width " << width << " Height " << height << " Pointer: " << ((long)ptr) << " Mat " << cv_image << endl;
    }
}
