// Based on slam_system.cc from apps in the original ptam-plus repository
// https://github.com/williammc/ptam_plus
//
// Rough-and-ready proof of concept of binding added, but...
// 
// Note this currently will NOT build due to 'cannot pop from empty stack'
// linker error (see README). Consequenrly it's untested.

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <emscripten.h>
#include <iostream>
#include <cvd/image.h>
#include <cvd/rgb.h>
#include <cvd/byte.h>
#include "ptam/track/tracker.h"
#include "ptam/construct/map_point.h"
#include <TooN/TooN.h>
#include <TooN/se3.h>
#include <emscripten/bind.h>
#include <vector>


using namespace emscripten;

double matrix[16];
std::vector<double> mapPoints;
        
extern "C" {
      void receiveData(uint8_t *ptr, int width, int height);
      void cleanup();
}

std::vector<double> getMapPoints();
double *getPoseMatrix();

ptam::Tracker *mpTracker;
ptam::Map *mpMap;
ptam::MapMaker *mpMapMaker;
ptam::ATANCamera *mpCamera;


EMSCRIPTEN_BINDINGS(my_class_example) {
    register_vector<double>("VectorDouble");
}

std::vector<double> getMapPoints() {
    return mapPoints;
}

double *getPoseMatrix() {
    return matrix;
}

EMSCRIPTEN_BINDINGS(return_data) {
    function("getMapPoints", &getMapPoints);
    function("getPoseMatrix", &getPoseMatrix, allow_raw_pointers());
}

int main(int argc, char *argv[]) {
    std::cout << "initialising ptam system..." << std::endl;
    mpCamera = new ptam::ATANCamera("camera");
    mpMap = new ptam::Map;
    mpMapMaker = new ptam::MapMaker (*mpMap, *mpCamera);
    
    mpTracker = new ptam::Tracker(CVD::ImageRef(400,300), *mpCamera, *mpMap, *mpMapMaker);

    matrix[15] = 1;
    matrix[12] = matrix[13] = matrix[14] = 0;

    return 0;
}

extern "C" void receiveData(uint8_t *ptr, int width, int height) {
    std::cout << "receiveData()" << std::endl;
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
    //    https://www.edwardrosten.com/cvd/toon/html-user/classTooN_1_1SE3.html
    // We can't get the raw matrix straight out of the SE3 so we have to
    // decompose it into translation and rotation components and then 
    // reconstruct the matrix.
    TooN::SE3<double> pose = mpTracker->GetCurrentPose();
    const TooN::Matrix<3,3,double> rotation = pose.get_rotation().get_matrix();
    for(int row=0; row<3; row++) {
        for(int col=0; col<3; col++) {
            matrix[row*4+col] = rotation(row, col);
        }    
    }

    TooN::Vector<3, double> translation = pose.get_translation();
    for(int i=0; i<3; i++) {
        matrix[i*4+3] = translation[i];
    }



    // the Map has a std::vector of pointers to MapPoint.
    // MapPoint contains a TooN::Vector v3WorldPos.
    mapPoints.clear();
    std::vector<ptam::MapPoint*> points = mpMap->points;
    mapPoints.reserve(points.size() * sizeof(double) * 3);
    
    for(int i=0; i<points.size(); i++) {
        for(int j=0; j<3; j++) {
            mapPoints.push_back(points[i]->v3WorldPos[j]);
        }
    }
}

extern "C"   void cleanup() {
    delete mpTracker;
    delete mpMapMaker;
    delete mpMap;
    delete mpCamera;
}
