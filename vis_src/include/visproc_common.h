#pragma once
#include "visproc_interface.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

#ifdef VISPROC_EXTENDED_TESTING
#include "opencv2/highgui.hpp"
#endif

extern bool scoresort(scoredContour c1, scoredContour c2);
extern double scoreDistanceFromTarget(const double target, double value);
extern double getFOVAngleHoriz(cv::Size targetSize, cv::Size fovSize, double distance);
extern double getFOVAngleVert(cv::Size targetSize, cv::Size fovSize, double distance);

extern std::pair<double, double> getRelativeAngleOffCenter(scoredContour object, cv::Size fovSize, double distance);
extern double getAngleOffCenterline(cv::Size targetSz);

#ifdef VISPROC_EXTENDED_TESTING
inline void drawOut(const char* window, cv::Mat out, bool live_out) {
	if(live_out) { cv::imshow(window, out); }
}
#else
inline void drawOut(const char* window, cv::Mat out, bool live_out) {}
#endif


const double cannyThresMin = 10;
const double cannyThresSize = 10;
const double pi = 3.14159265358979323846;

extern int goal_hueThres[2];
extern int goal_valThres[2];

extern int ball_hueThres[2];
extern int ball_satThres[2];
extern int ball_valThres[2];
