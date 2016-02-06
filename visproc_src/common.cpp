#include "visproc_common.h"
#include "visproc_interface.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

const double fovHoriz = 0.776417; // rad
const double fovVert = 0.551077; // rad

const double targetWidth = 20.0; // inches
const double targetHeight = 14.0; // inches

bool scoresort(scoredContour c1, scoredContour c2) {
        return (c1.first < c2.first);
}

double scoreDistanceFromTarget(const double target, double value) {
        double distanceRatio = (fabs(target - fabs(target - value)) / target);
        return fmax(0, fmin(distanceRatio*100, 100));
}

/* fovWidth = width of input image in pixels */
double getDistance(cv::Size targetSize, cv::Size fovSize) {
	double dW = targetWidth * fovSize.width / (targetSize.width * tan(fovHoriz));
	//double dH = targetHeight * fovSize.height / (targetSize.height * tan(fovVert));

	return dW;

	//return ((dW + dH) / 2.0); // returns feet
}

double getFOVAngleHoriz(cv::Size targetSize, cv::Size fovSize, double distance) {
	return atan2(targetWidth * fovSize.width, targetSize.width * distance);
}

double getFOVAngleVert(cv::Size targetSize, cv::Size fovSize, double distance) {
	return atan2(targetHeight * fovSize.height, targetSize.height * distance);
}

/* returns angle off center horizontal and angle off center vertical */
std::pair<double, double> getRelativeAngleOffCenter(scoredContour object, cv::Size fovSize, double distance) {
	std::pair<double, double> out;

	cv::Moments m = cv::moments(object.second);
	double cX = m.m10 / m.m00;
	double cY = m.m01 / m.m00;
	
	double xAxis = fovSize.height / 2;
	double yAxis = fovSize.width / 2;

	double dX = abs(yAxis - cX);
	double dY = abs(xAxis - cY);
	
	if(cX < yAxis) { // left of center
		out.first = asin(dX / distance);
	} else {
		out.first = -asin(dX / distance);
	}

	if(cY < xAxis) { // above center
		out.second = asin(dY / distance);
	} else {
		out.second = -asin(dY / distance);
	}

	return out;
}
