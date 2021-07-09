// Based on slam_system.cc from apps in the original ptam-plus repository
// https://github.com/williammc/ptam_plus
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <emscripten.h>
#include <iostream>
#include "cvd/image.h"
#include "cvd/rgb.h"
#include "cvd/byte.h"
#include "ptam/track/tracker.h"


using namespace std;
using namespace ptam;

extern "C" {
     EMSCRIPTEN_KEEPALIVE void receiveData(uint8_t *ptr, int width, int height);
     EMSCRIPTEN_KEEPALIVE void cleanup();
}

Tracker *mpTracker;
Map *mpMap;
MapMaker *mpMapMaker;
ATANCamera *mpCamera;

int main(int argc, char *argv[]) {
    cout << "initialising ptam system..." << endl;
    mpCamera = new ATANCamera("camera");
    mpMap = new Map;
    mpMapMaker = new MapMaker(*mpMap, *mpCamera);
    mpTracker = new Tracker(CVD::ImageRef(800,600), *mpCamera, *mpMap, *mpMapMaker);
    return 0;
}

extern "C"  EMSCRIPTEN_KEEPALIVE void receiveData(uint8_t *ptr, int width, int height) {
    cout << "receiveData()" << endl;
    auto cv_image = cv::Mat(width, height, CV_8UC4, ptr);
    cv::Mat gray_image;
    CVD::Image<CVD::byte> frameBW;
    CVD::Image<CVD::Rgb<CVD::byte> > frameRGB;
    
    // From PTAM code (slam_system.cc)
    // TODO - Not sure if this is needed here
    //cv::cvtColor(cv_image, cv_image, CV_BGR2RGB);

    cv::cvtColor(cv_image, gray_image, cv::COLOR_RGB2GRAY);
    

    CVD::SubImage<CVD::Rgb<CVD::byte> > cvd_rgb_frame(
        (CVD::Rgb<CVD::byte>*)cv_image.data,
        CVD::ImageRef(cv_image.cols, cv_image.rows), cv_image.cols);
    CVD::SubImage<CVD::byte> cvd_gray_frame(gray_image.data,
            CVD::ImageRef(gray_image.cols, gray_image.rows), cv_image.cols);

    frameBW.copy_from(cvd_gray_frame);
    frameRGB.copy_from(cvd_rgb_frame);

    mpTracker->TrackFrame(frameBW, true);
}

extern "C"  EMSCRIPTEN_KEEPALIVE void cleanup() {
    delete mpTracker;
    delete mpMapMaker;
    delete mpMap;
    delete mpCamera;
}
