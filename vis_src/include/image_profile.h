#pragma once
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <vector>

extern std::vector<unsigned int> particleHeightProfile(std::vector<cv::Point> contour) ;

// profile analysis:
// Change of > 10% of total contour height : edge
// general sequence: high state - falling edge - low state - rising edge - high state
// Relevant parameters:
//  length of low state
//  difference between high and low states

struct goalProfileData {
	unsigned int fallingEdgeStart;
	unsigned int fallingEdgeEnd;

	unsigned int risingEdgeStart;
	unsigned int risingEdgeEnd;

	unsigned int averageHighState1;
	unsigned int averageLowState;
	unsigned int averageHighState2;
};

// half the size of the sliding window, rounded down.
// this gives us a total sliding window of 2(2) + 1 = 5 (1 central value and 2 values on either side)
const unsigned int prof_HalfSMA = 2;
const double profileEdgeThreshold = 0.10;

extern goalProfileData analyzeHeightProfile(std::vector<unsigned int> profile, unsigned int contourHeight)
