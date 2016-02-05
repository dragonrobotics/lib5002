#pragma once

#include "visproc_interface.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

extern bool scoresort(scoredContour c1, scoredContour c2);
extern double scoreDistanceFromTarget(const double target, double value);
extern double getDistance(cv::Size targetSize, cv::Size fovSize);
extern double getFOVAngleHoriz(cv::Size targetSize, cv::Size fovSize, double distance);
extern double getFOVAngleVert(cv::Size targetSize, cv::Size fovSize, double distance);
extern std::pair<double, double> getRelativeAngleOffCenter(scoredContour object, cv::Size fovSize, double distance);
