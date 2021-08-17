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


//using namespace emscripten;
using std::cout;
using std::endl;
using std::vector;

double poseMatrix[16];

class PoseMatrix {
    public:
        double *matrix;

        emscripten::val get(int index) {
            if(matrix == NULL) {
                return emscripten::val::undefined();
            } else if(index >= 0 && index < 16) {
                return emscripten::val(matrix[index]);
            } else {
                return emscripten::val::undefined();
            }
        }

        void loadLatestMatrix() {
            matrix = poseMatrix;
        }
}; 

vector<double> mapPoints;
        
extern "C" {
      EMSCRIPTEN_KEEPALIVE int receiveData(uint8_t *ptr, int width, int height);
      EMSCRIPTEN_KEEPALIVE void cleanup();
      EMSCRIPTEN_KEEPALIVE void captureKeyFrame();
}

vector<double> getMapPoints();

ptam::Tracker *mpTracker;
ptam::Map *mpMap;
ptam::MapMaker *mpMapMaker;
ptam::ATANCamera *mpCamera;

bool isProcessingFrame = false;

vector<double> getMapPoints() {
    return mapPoints;
}

EMSCRIPTEN_BINDINGS(map_points) {
    emscripten::function("getMapPoints", &getMapPoints);
    emscripten::register_vector<double>("vector<double>");
}

EMSCRIPTEN_BINDINGS(pose_matrix) {
    emscripten::class_<PoseMatrix>("PoseMatrix")
        .constructor()
        .function("loadLatestMatrix", &PoseMatrix::loadLatestMatrix)
        .function("get", &PoseMatrix::get);
}


int main(int argc, char *argv[]) {
    cout << "initialising ptam system..." << endl;
    
    mpCamera = new ptam::ATANCamera("Camera");
    
    mpMap = new ptam::Map;
    mpMapMaker = new ptam::MapMaker (*mpMap, *mpCamera);
    
    mpTracker = new ptam::Tracker(CVD::ImageRef(800, 600), *mpCamera, *mpMap, *mpMapMaker);

    
    poseMatrix[15] = 1;
    poseMatrix[12] = poseMatrix[13] = poseMatrix[14] = 0;
    

    return 0;
}

extern "C" EMSCRIPTEN_KEEPALIVE int receiveData(uint8_t *ptr, int width, int height) {

//    cout << "receiveData()" << endl;
    if(!isProcessingFrame) {
        isProcessingFrame = true;
        auto cv_image = cv::Mat(width, height, CV_8UC4, ptr);
    
        cv::Mat gray_image;
        CVD::Image<CVD::byte> frameBW;
        CVD::Image<CVD::Rgb<CVD::byte> > frameRGB;
    
        // From PTAM code (slam_system.cc)
        // TODO - Not sure if this is needed here
        //cv::cvtColor(cv_image, cv_image, cv::COLOR_BGR2RGB);

        cv::cvtColor(cv_image, gray_image, cv::COLOR_RGB2GRAY);
    

        CVD::SubImage<CVD::Rgb<CVD::byte> > cvd_rgb_frame(
            (CVD::Rgb<CVD::byte>*)cv_image.data,
            CVD::ImageRef(cv_image.cols, cv_image.rows), cv_image.cols);
        CVD::SubImage<CVD::byte> cvd_gray_frame(gray_image.data,
            CVD::ImageRef(gray_image.cols, gray_image.rows), cv_image.cols);

        frameBW.copy_from(cvd_gray_frame);
        frameRGB.copy_from(cvd_rgb_frame);


        // This will try to get the tracking going, first stage is to get two keyframes, first keyframe is obtained when we 'press space' (simulated by call above), second is when we 'press space' again

        ptam::Tracker::ResetStatus resetStatus = mpTracker->TrackFrame(frameBW, true);
        //    https://www.edwardrosten.com/cvd/toon/html-user/classTooN_1_1SE3.html
        // We can't get the raw matrix straight out of the SE3 so we have to
        // decompose it into translation and rotation components and then 
        // reconstruct the matrix.




        TooN::SE3<double> pose = mpTracker->GetCurrentPose();
        const TooN::Matrix<3,3,double> rotation = pose.get_rotation().get_matrix();
        for(int row=0; row<3; row++) {
            for(int col=0; col<3; col++) {
                poseMatrix[row*4+col] = rotation(row, col);
            }    
        }

        TooN::Vector<3, double> translation = pose.get_translation();
        for(int i=0; i<3; i++) {
            poseMatrix[i*4+3] = translation[i];
        }
        /*
        cout << "Matrix: ";
        for(int i=0; i<16; i++) {
           if(!(i%4)) {
               cout << endl;
           }
           cout << poseMatrix[i] << " ";
        }
        cout << endl;
        */



        // the Map has a vector of pointers to MapPoint.
        // MapPoint contains a TooN::Vector v3WorldPos.
        mapPoints.clear();
        vector<ptam::MapPoint*> points = mpMap->points;
        mapPoints.reserve(points.size() * sizeof(double) * 3);
    
        for(int i=0; i<points.size(); i++) {
            cout << " worldpos for point " << i << " : ";
            for(int j=0; j<3; j++) {
                mapPoints.push_back(points[i]->v3WorldPos[j]);
                cout << points[i]->v3WorldPos[j]  << " ";
            }
            cout << endl;
        }
        isProcessingFrame = false;
        return (resetStatus == ptam::Tracker::RESET) ? 0: (resetStatus == ptam::Tracker::BOTH_FRAMES_CAPTURED ? 2: 1);
    } else {
        cout << "$$$$ Frame ignored as still processing previous frame!!!" << endl;
        return 1;
    }
}

// simulate the 'space press'...
extern "C" EMSCRIPTEN_KEEPALIVE void captureKeyFrame() {
    cout << "### ptam_wasm.cpp: simulating space press" << endl;
    mpTracker->AskInitialTrack();
}

extern "C"   EMSCRIPTEN_KEEPALIVE void cleanup() {
    delete mpTracker;
    delete mpMapMaker;
    delete mpMap;
    delete mpCamera;
}
