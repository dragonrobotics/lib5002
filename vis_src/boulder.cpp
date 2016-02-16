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

int ball_hueThres[2] = {0, 180};
int ball_satThres[2] = {0, 35};
int ball_valThres[2] = {80, 173};
const double area_threshold = 500;

cv::Mat boulder_preprocess_pipeline(cv::Mat input, bool suppress_output, bool live_output) {
        cv::Mat tmp(input.size(), input.type());

        cv::cvtColor(input, tmp, CV_BGR2HSV);

        /* Make things easier for the HV filter */
        //cv::blur(tmp, tmp, cv::Size(5,5));
        cv::GaussianBlur(tmp, tmp, cv::Size(3,3), 1.5, 1.5, cv::BORDER_DEFAULT);
		drawOut("stage1", tmp, live_output);

        /* Filter on saturation and brightness */
        cv::Mat mask(input.size(), CV_8U);
        cv::inRange(tmp,
					cv::Scalar((unsigned char)ball_hueThres[0],(unsigned char)ball_satThres[0],(unsigned char)ball_valThres[0]),
					cv::Scalar((unsigned char)ball_hueThres[1],(unsigned char)ball_satThres[1],(unsigned char)ball_valThres[1]),
					mask);
		drawOut("stage2", mask, live_output);

        /* Dilate away smaller hits */
        cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5)));
		drawOut("stage3", mask, live_output);

        /* Blur for edge detection */
        cv::Mat edgedet;
        cv::blur(mask, edgedet, cv::Size(7,7));
		drawOut("stage4", edgedet, live_output);

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
		drawOut("stage5", edgedet, live_output);

        return edgedet;
}

std::vector<scoredContour> boulder_pipeline(cv::Mat input, bool suppress_output, bool window_output) {
    std::vector< std::vector<cv::Point> > contours;

    cv::Mat contourOut = input.clone();

    cv::findContours(contourOut, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    std::vector<scoredContour> finalscores;

    std::cout << "Found " << contours.size() << " contours." << std::endl;

	if(window_output) {
		cv::Mat conOut = cv::Mat::zeros(input.size(), CV_8UC3);
		for(int i=0;i<contours.size();i++) {
			double area = cv::contourArea(contours[i]);
			if(area < area_threshold) {
                continue;
            }
			cv::Scalar col(rand()&180, rand()&255, rand()&255);
			cv::drawContours(conOut, contours, i, col, CV_FILLED, 8);
		}
		drawOut("contours", conOut, window_output);
	}

    unsigned int ctr = 0;
    for(std::vector< std::vector<cv::Point> >::iterator i = contours.begin();
        i != contours.end(); ++i) {
            ctr++;
            double area = cv::contourArea(*i);
            double perimeter = cv::arcLength(*i, true);
            cv::Rect bounds = cv::boundingRect(*i);

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

            double idealRadius = (bounds.width/2);
            double idealArea = pi * (idealRadius * idealRadius);

            double circularity = scoreDistanceFromTarget(idealArea, area);
            double ar_score = scoreDistanceFromTarget(1, bounds.width / bounds.height);

            double total_score = (circularity + ar_score) / 2;
	
			if(!suppress_output) {
		        std::cout << "Circularity Score: " << circularity << std::endl;
		        std::cout << "AsRatio Score: " << ar_score << std::endl;
		        std::cout << "Total Score: " << total_score << std::endl;
			}

            finalscores.push_back(std::make_pair(total_score, std::move(*i)));
    }

	if(finalscores.size() > 0) {
    	std::sort(finalscores.begin(), finalscores.end(), &scoresort);
		std::reverse(finalscores.begin(), finalscores.end());

    	return finalscores;
	} else {
		std::vector<scoredContour> ret;
		ret.push_back(std::make_pair(0.0, std::vector<cv::Point>()));
		return ret;
	}
}
