#include "image_profile.h"

// calculate 1-dimensional height profile of a given contour.
// (y = distance(lowestpoint(x), highestpoint(x)) for all columns x
std::vector<unsigned int> particleHeightProfile(std::vector<cv::Point> contour) {
	cv::Rect bounds = cv::boundingRect(contour);
	std::vector<unsigned int> distance(bounds.width, 0); // smallest distance from top of bounds to point in column of contour
	
	int bottomHeight = (bounds.y + bounds.height);

	// find lowest / highest points per column in contour:
	for(cv::Point i : contour) {
		int col = i.x -  bounds.x;
		int dist = abs(bottomHeight - i.y);
		distance[col] = max(distance[col], dist);
	}

	return distance;
}

goalProfileData analyzeHeightProfile(std::vector<unsigned int> profile, unsigned int contourHeight) {
	// A sliding window algorithm is used to detect edges here:
	
	goalProfileData data;

	unsigned int slidingWindowMean = 0;
	for(unsigned int i=0;i<prof_HalfSMA;i++) {
		slidingWindowMean += profile[i];
	}
	slidingWindowMean /= prof_HalfSMA;

	unsigned int thres = (((double)contourHeight) * profileEdgeThreshold);

	bool inEdge = false;
	bool falling = false;
	bool fallingFound = false;
	for(unsigned int i=0;i<profile.length();i++) {
		int begin = max(i-prof_HalfSMA, 0);
		int end = min(i+prof_HalfSMA, profile.length());
		// profile[end] is added to our window
		// profile[begin] is dropped from our window
		int nElem = end - begin;
		slidingWindowMean += ((profile[end] / nElem) - (profile[begin] / nElem));

		unsigned int diff = abs(profile[end] - slidingWindowMean);

		if(!inEdge) {
			if(diff > thres) { // start of edge
				inEdge = true;
				if(!fallingFound) {
					data.fallingEdgeStart = end;
					fallingFound = true;
					falling = true;
				} else {
					data.risingEdgeStart = end;
				}
			}
		} else {
			if(diff < thres) { // end of edge
				inEdge = false;
				if(falling) {
					data.fallingEdgeEnd = end;
					falling = false;
				} else {
					data.risingEdgeEnd = end;
				}
			}
		}
	}

	// calculate more data:
	for(unsigned int i=0;i<data.fallingEdgeStart;i++) {
		data.averageHighState1 += profile[i];
	}
	data.averageHighState1 /= data.fallingEdgeStart;
	
	for(unsigned int i=data.fallingEdgeEnd;i<data.risingEdgeStart;i++) {
		data.averageLowState += profile[i];
	}
	data.averageLowState /= data.risingEdgeStart - data.fallingEdgeEnd;

	for(unsigned int i=data.risingEdgeEnd;i<profile.length();i++) {
		data.averageHighState2 += profile[i];
	}
	data.averageHighState2 /= data.risingEdgeEnd - profile.length();

	return data;
}
