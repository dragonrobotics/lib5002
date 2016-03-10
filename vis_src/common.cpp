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

bool scoresort(scoredContour c1, scoredContour c2) {
        return (c1.first < c2.first);
}

double scoreDistanceFromTarget(const double target, double value) {
        double distanceRatio = (fabs(target - fabs(target - value)) / target);
        return fmax(0, fmin(distanceRatio*100, 100));
}

/* fovWidth = width of input image in pixels */
double getDistance(cv::Size observedSize, cv::Size targetSize, cv::Size fovSize) {
	double dW = (targetSize.width * fovSize.width) / (observedSize.width * tan(fovHoriz));
	//double dH = targetSize.height * fovSize.height / (observedSize.height * tan(fovVert));

	return dW;

	//return ((dW + dH) / 2.0); // returns feet
}

double getDistance(double observedHeight, double targetHeight, double frameHeight, double fovAngle) {
	return (targetHeight * frameHeight) / (observedHeight * tan(fovAngle)); // tan(fovAngle) = (frameHeight[px/ft] / distance[px/ft]);
}

double getAngleOffCenter(double midpointX, double frameWidth, double fovAngle) {
	return ((midpointX - (frameWidth/2)) / frameWidth) * (fovAngle/2);
}

double getFOVAngleHoriz(cv::Size observedSize, cv::Size targetSize, cv::Size fovSize, double distance) {
	return atan2(targetSize.width * fovSize.width, observedSize.width * distance);
}

double getFOVAngleVert(cv::Size observedSize, cv::Size targetSize, cv::Size fovSize, double distance) {
	return atan2(targetSize.height * fovSize.height, observedSize.height * distance);
}

double getAngleOffCenterline(cv::Size targetSz, cv::Size obsSz) {
	double AStgt = obsSz.width / obsSz.height;
	
	double ASdiff = ((obsSz.width - 1) / obsSz.height) / (targetSz.width / targetSz.height);  //  ((AStgt - (1 / obsSz.height)) / (obsSz.width / obsSz.height));

	return 90.0 * ASdiff;
}
