#include "visproc_common.h"
#include "visproc_interface.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

const int camID = 1;

#ifdef VISPROC_BASIC_TESTING
int main(int argc, char** argv) {
	cv::VideoCapture cap(camID); // open cam 1
	if(!cap.isOpened())  // check if we succeeded
		return -1;

	while(true) {
		cv::Mat src;
		if( !cap.read(src) ) {
			std::cerr << "Error reading image from camera";	
			return -1;
		}

		scoredContour out = goal_pipeline(goal_preprocess_pipeline(src));

		if( out.second.size() > 0 ) {
			cv::Rect bounds = cv::boundingRect(out.second);
			double dist = getDistance(bounds.size(), src.size());
			std::cout << "Distance: " << std::to_string(dist) << " inches" << std::endl;
		}
	}
}
#endif

#ifdef VISPROC_EXTENDED_TESTING

#ifdef VISPROC_BALL_TEST
int main(int argc, char** argv) {
		cv::namedWindow("input");
		cv::namedWindow("output");

		cv::VideoCapture cap(1); // open cam 1
		if(!cap.isOpened())  // check if we succeeded
			return -1;

		cv::namedWindow("stage1");
		cv::namedWindow("stage2");
		cv::namedWindow("stage3");
		cv::namedWindow("stage4");
		cv::namedWindow("stage5");
		cv::namedWindow("contours");

		cvCreateTrackbar("Hue Min", "input", &(ball_hueThres[0]), 179, NULL);
		cvCreateTrackbar("Hue Max", "input", &(ball_hueThres[1]), 179, NULL);

		cvCreateTrackbar("Sat Min", "input", &(ball_satThres[0]), 255, NULL);
		cvCreateTrackbar("Sat Max", "input", &(ball_satThres[1]), 255, NULL);

		cvCreateTrackbar("Val Min", "input", &(ball_valThres[0]), 255, NULL);
		cvCreateTrackbar("Val Max", "input", &(ball_valThres[1]), 255, NULL);

		std::vector<cv::Point> last_good;

		while(true) {
			cv::Mat src;
			if( !cap.read(src) ) {
				std::cerr << "Error reading image from camera";	
				return -1;
			}

			cv::imshow("input", src);

			std::vector<scoredContour> out = boulder_pipeline(boulder_preprocess_pipeline(src, true, true), true, true);

			cv::Mat output = cv::Mat::zeros(src.size(), CV_8UC3);

			std::vector< std::vector<cv::Point> > drawVec;

			for(std::vector<scoredContour>::iterator it = out.begin();it != out.end();++it) {
				if(it->first < 85.0)
					continue;

				drawVec.push_back(it->second);
				cv::Rect bounds = cv::boundingRect(it->second);

				cv::Point center(bounds.tl().x+(bounds.width / 2), bounds.tl().y+(bounds.height/2));

				double distance = getDistance(bounds.size(), src.size());

				cv::putText(output, std::to_string(distance) + " inches", center, cv::FONT_HERSHEY_PLAIN, 0.25, cv::Scalar(0, 0, 255));
			}

			cv::drawContours(output, drawVec, 0, cv::Scalar(255,255,255));

			cv::imshow("output", output);

			if(cv::waitKey(30) >= 0) { 
				break;
			}
		}
}
#endif

#ifdef VISPROC_GOAL_TEST
int main(int argc, char** argv) {
		cv::namedWindow("input");
		cv::namedWindow("output");

		cv::VideoCapture cap(camID); // open cam 1
		if(!cap.isOpened())  // check if we succeeded
			return -1;

		cv::namedWindow("stage1");
		cv::namedWindow("stage2");
		cv::namedWindow("stage3");
		cv::namedWindow("stage4");
		cv::namedWindow("stage5");
		cv::namedWindow("contours");

		cvCreateTrackbar("Hue Min", "input", &(goal_hueThres[0]), 179, NULL);
		cvCreateTrackbar("Hue Max", "input", &(goal_hueThres[1]), 179, NULL);

		cvCreateTrackbar("Val Min", "input", &(goal_valThres[0]), 255, NULL);
		cvCreateTrackbar("Val Max", "input", &(goal_valThres[1]), 255, NULL);

		std::vector<cv::Point> last_good;

		while(true) {
			cv::Mat src;
			if( !cap.read(src) ) {
				std::cerr << "Error reading image from camera";	
				return -1;
			}

			cv::imshow("input", src);

			double t = (double)cv::getTickCount();

			scoredContour out = goal_pipeline(goal_preprocess_pipeline(src, true, true), true);

			double fps = 1 / (((double)cv::getTickCount() - t) / cv::getTickFrequency());

			std::cout << "FPS: " << fps << std::endl;
			cv::Mat output = cv::Mat::zeros(src.size(), CV_8UC3);

			if( out.second.size() > 0 ) {
				std::vector< std::vector<cv::Point> > drawVec;
				drawVec.push_back(out.second);

				cv::Rect bounds = cv::boundingRect(out.second);
				last_good = out.second;

				double distance = getDistance(bounds.size(), src.size());
				std::pair<double, double> angles = getRelativeAngleOffCenter(out, bounds.size(), distance);
				double angleToTarget = getAngleOffCenterline(bounds.size());

				cv::Scalar col(255,255,255);
				cv::drawContours(output, drawVec, 0, col);

				cv::putText(output, std::to_string(fps), cv::Point(50, 50), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 0, 255));

				cv::putText(output, std::to_string(distance) + " inches", cv::Point(50, 75), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 255, 0));

				cv::putText(output, std::string("AoT Horizontal: ") + std::to_string(angles.first * (180.0 / pi)) + " degrees", cv::Point(50, 75), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 255, 0));

				cv::putText(output, std::string("AoT Vertical ") + std::to_string(angles.second  * (180.0 / pi)) + " degrees", cv::Point(50, 100), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 0, 0));

				cv::putText(output, std::string("AoCenterLine Horiz: ") + std::to_string(angleToTarget) + " degrees", cv::Point(50, 125), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 0, 0));
			
				cv::imshow("output", output);
			}

			if(cv::waitKey(30) >= 0) { 
				break;
			}
		}
}
#endif

#endif
