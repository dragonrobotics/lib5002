#include "visproc_interface.h"
#include "visproc_common.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

/*! \file goal.cpp
 *  \brief Contains goal processing functions (for 2016)
 */

const cv::Size goalSz(20.0, 12.0); // width, height -- TODO: Make sure this is correct!
const double goalAS = goalSz.width / goalSz.height;

typedef std::pair<double, double> distancePair;
typedef std::pair<double, double> anglePair;

const double fovHoriz = 0.776417; // rad
const double fovVert = 0.551077; // rad

// get horizontal angles to goal sides: left then right
anglePair getAnglesToGoalSides(scoredContour contour, cv::Size frameSize) {
	anglePair ret;
	cv::Rect boundingBox = cv::boundingRect(contour.second)
	cv::Point bl = cv::Point(boundingBox.x, boundingBox.y+boundingBox.height); // bottom left corner
	
	ret.first = ((2*((frameSize.width/2)-boundingBox.x)) / frameSize.width) * fovHoriz;
	ret.second = ((2*((frameSize.width/2)-boundingBox.br().x)) / frameSize.width) * fovHoriz;
	
	return ret;
}

// get vertical distances to goal bottom and top (in order)
distancePair getDistancesToGoalSides(scoredContour contour, cv::Size frameSize) {
	distancePair ret;
	cv::Rect boundingBox = cv::boundingRect(contour.second)
	cv::Point bl = cv::Point(boundingBox.x, boundingBox.y+boundingBox.height); // bottom left corner
	
	
	double theta = (boundingBox.height / frameSize.height) * fovVert;
	
	ret.first = (goalSz.height / tan(theta));
	ret.second = (goalSz.height / sin(theta));
	
	return ret;
}

/*
TODO:

 * Motion / Differential analysis? 
 *
 * Return:
 * - Robot position relative to target. 
 *  > Distance
 *  > Angle left / right (0=centered)
 * 
 * Angle left / right:
 * - Find intensity profile?
 * - Calculate observed aspect ratio vs ideal aspect ratio?

 * theta = angle off center horizontally, rho = angle off center vertically
 
 * - At theta = 0, observed aspect ratio = ideal aspect ratio
 * - Observed aspect ratio goes to (1 / height) as angle off centerline goes to +-90.
 * - Ratio between ASes goes to ((1/height) / (ideal width / ideal height)) = (1 / ideal width) as theta goes to +- 90.

 * - At rho = 0, observed aspect ratio = ideal aspect ratio
 * - Observed AS goes to (width) as rho goes to +- 90
 * - Ratio between ASes goes to (width / (width / height)) = (height) as rho goes to +- 90.

 * Send on-target alert.
 */

int goal_hueThres[2] = {70, 100};	//!< Hue thresholds (min, max) for detecting goal retroreflective tape.
int goal_valThres[2] = {128, 255};	//!< Value thesholds for detecting goals.

/*!	\fn goal_preprocess_pipeline(cv::Mat input, bool suppress_output, bool live_output)
 *	\brief Filter and edge-detect an image, searching for goals.
 *
 *	\param input Input frame.
 *	\param suppress_output If false, then debugging data is written to stdout.
 *	\param live_output If true, then intermediate pipeline image streams are output to GUI windows.
 */
cv::Mat goal_preprocess_pipeline(cv::Mat input, bool suppress_output, bool live_output) {
        cv::Mat tmp(input.size(), input.type());

        cv::cvtColor(input, tmp, CV_BGR2HSV);

        /* Make things easier for the HV filter */
        //cv::blur(tmp, tmp, cv::Size(5,5));
        cv::GaussianBlur(tmp, tmp, cv::Size(5,5), 2.5, 2.5, cv::BORDER_REPLICATE);
		drawOut("stage1", tmp, live_output);

        /* Filter on color/brightness */
        cv::Mat mask(input.size(), CV_8U);
        cv::inRange(tmp,
					cv::Scalar((unsigned char)goal_hueThres[0],0,(unsigned char)goal_valThres[0]),
					cv::Scalar((unsigned char)goal_hueThres[1],255,(unsigned char)goal_valThres[1]),
					mask);
		drawOut("stage2", mask, live_output);

        /* Erode away smaller hits */
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5)));
		drawOut("stage3", mask, live_output);

        /* Blur for edge detection */
        cv::Mat edgedet;
        cv::blur(mask, edgedet, cv::Size(3,3));
		drawOut("stage4", edgedet, live_output);

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
		drawOut("stage5", edgedet, live_output);        

		return edgedet;
}

/*!	\fn goal_pipeline(cv::Mat input, bool suppress_output, bool live_output)
 *	\brief Score and retrieve the best seeming goal from a preprocessed image.
 *
 *	\param input Input frame.
 *	\param suppress_output If false, then debugging data is written to stdout.
 *	\param live_output If true, then intermediate pipeline image streams are output to GUI windows.
 */
scoredContour goal_pipeline(cv::Mat input, bool suppress_output, bool window_output) {
        std::vector< std::vector<cv::Point> > contours;

        cv::Mat contourOut = input.clone();

        cv::findContours(contourOut, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        std::vector<scoredContour> finalscores;

        if(!suppress_output) { std::cout << "Found " << contours.size() << " contours." << std::endl; }

		if(window_output) {
			cv::Mat conOut = cv::Mat::zeros(input.size(), CV_8UC3);
			for(int i=0;i<contours.size();i++) {
				double area = cv::contourArea(contours[i]);
				cv::Scalar col(rand()&180, rand()&255, rand()&255);
				cv::drawContours(conOut, contours, i, col, CV_FILLED, 8);
			}
			drawOut("contours", conOut, window_output);
		}

        unsigned int ctr = 0;
        for(std::vector< std::vector<cv::Point> >::iterator i = contours.begin();
            i != contours.end(); ++i) {
                double area = cv::contourArea(*i);
                double perimeter = cv::arcLength(*i, true);
                cv::Rect bounds = cv::boundingRect(*i);

                const double cvarea_target = (80.0 / (goalSz.width * goalSz.height)); //(80.0/240.0);
                const double area_threshold = 1000;

                /*! Area Thresholding Test
                 * Only accept contours of a certain total size.
                 */

                if(area < area_threshold) {
                    continue;
                }

		if(!suppress_output) {
			std::cout << std::endl;
			std::cout << "Contour " << ctr << ": " << std::endl;
			ctr++;
			std::cout << "Area: "  << area << std::endl;
			std::cout << "Perimeter: " << perimeter << std::endl;
		}

                /*! Coverage Area Test
                 * Compare particle area vs. Bounding Rectangle area.
                 * score = 1 / abs((1/3)- (particle_area / boundrect_area))
                 * Score decreases linearly as coverage area tends away from 1/3. */
                double cvarea_score = 0;

                double coverage_area = area / bounds.area();
                cvarea_score = scoreDistanceFromTarget(cvarea_target, coverage_area);

                /*! Aspect Ratio Test
                 * Computes aspect ratio of detected objects.
                 */

                double tmp = bounds.width;
                double aspect_ratio = tmp / bounds.height;
                double ar_score = scoreDistanceFromTarget(goalAS, aspect_ratio);

                /*! Image Moment Test
                 * Computes image moments and compares it to known values.
                 */

                cv::Moments m = cv::moments(*i);
                double moment_score = scoreDistanceFromTarget(0.28, m.nu02);

                /*! Image Orientation Test
                 * Computes angles off-axis or contours.
                 */
                // theta = (1/2)atan2(mu11, mu20-mu02) radians
                // theta ranges from -90 degrees to +90 degrees.
                double theta = (atan2(m.mu11,m.mu20-m.mu02) * 90) / pi;
                double angle_score = (90 - fabs(theta))+10;

		if(!suppress_output) {
		    std::cout << "nu-02: " << m.nu02 << std::endl;
		    std::cout << "CVArea: "  <<  coverage_area << std::endl;
		    std::cout << "AsRatio: " << aspect_ratio << std::endl;
		    std::cout << "Orientation: " << theta << std::endl;
		}

                double total_score = (moment_score + cvarea_score + ar_score + angle_score) / 4;

		if(!suppress_output) {
		    std::cout << "CVArea Score: "  <<  cvarea_score << std::endl;
		    std::cout << "AsRatio Score: " << ar_score << std::endl;
		    std::cout << "Moment Score: " << moment_score << std::endl;
		    std::cout << "Angle Score: " << angle_score << std::endl;
		    std::cout << "Total Score: " << total_score << std::endl;
		}

                finalscores.push_back(std::make_pair(total_score, std::move(*i)));
        }

       if(finalscores.size() > 0) {
		std::sort(finalscores.begin(), finalscores.end(), &scoresort);

		return finalscores.back();
	} else {
		return std::make_pair(0.0, std::vector<cv::Point>());	
	}
}
