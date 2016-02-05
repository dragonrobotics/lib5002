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

#ifdef VISPROC_BASIC_TESTING
int main(int argc, char** argv) {
	cv::VideoCapture cap(1); // open cam 1
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
		}
	}
}
#endif

#ifdef VISPROC_EXTENDED_TESTING
int main(int argc, char** argv) {
        if(argc == 1) {
            std::cout << "Need input file to process." << std::endl;
            return 1;
        }
        std::string infile = argv[1];
        bool boulderproc = false;
		bool videoproc = false;

        for(int i=2;i<argc;i++) {           
			std::string opt = argv[i];
	        if(opt == "--boulder") {
	            boulderproc = true;
	        } else if(opt == "--video") {
				videoproc = true;		
			}
		}

		cv::namedWindow("input");
		cv::namedWindow("output");

		if(!videoproc) {
			cv::Mat src;			

			if(infile == "video") {
				cv::VideoCapture cap(0);
				if(!cap.isOpened())
					return -1;
				
				if( !cap.read(src) ) {
					std::cerr << "Error reading image from camera";	
					return -1;
				}
			} else {
				src = cv::imread(infile, 1);
			}
			

			scoredContour out;
			if(!boulderproc) {
			    out = goal_pipeline(goal_preprocess_pipeline(src));
			} else {
			    out = boulder_pipeline(boulder_preprocess_pipeline(src));
			}

			cv::Mat output = cv::Mat::zeros(src.size(), CV_8UC3);

			std::vector< std::vector<cv::Point> > drawVec;
			drawVec.push_back(out.second);

			cv::Scalar col(255,255,255);
			cv::drawContours(output, drawVec, 0, col);

			cv::imwrite("pipeline_output.png", output);

			cv::imshow("input", src);
			cv::imshow("output", output);

			while(true) {
				if(cv::waitKey(30) >= 0) break;
			}
		} else {
			cv::VideoCapture cap(1); // open cam 1
			if(!cap.isOpened())  // check if we succeeded
				return -1;

			cv::namedWindow("stage1");
			cv::namedWindow("stage2");
			cv::namedWindow("stage3");
			cv::namedWindow("stage4");
			cv::namedWindow("stage5");

			cvCreateTrackbar("Hue Min", "input", &(hueThres[0]), 179, NULL);
			cvCreateTrackbar("Hue Max", "input", &(hueThres[1]), 179, NULL);

			cvCreateTrackbar("Val Min", "input", &(valThres[0]), 255, NULL);
			cvCreateTrackbar("Val Max", "input", &(valThres[1]), 255, NULL);

			std::vector<cv::Point> last_good;

			while(true) {
				cv::Mat src;
				if( !cap.read(src) ) {
					std::cerr << "Error reading image from camera";	
					return -1;
				}

				cv::imshow("input", src);

				double t = (double)cv::getTickCount();

				scoredContour out;
				if(!boulderproc) {
					out = goal_pipeline(goal_preprocess_pipeline(src, true, true), true);
				} else {
					out = boulder_pipeline(boulder_preprocess_pipeline(src, true, true), true);
				}

				double fps = 1 / (((double)cv::getTickCount() - t) / cv::getTickFrequency());

				std::cout << "FPS: " << fps << std::endl;
				cv::Mat output = cv::Mat::zeros(src.size(), CV_8UC3);

				if( out.second.size() > 0 ) {
					std::vector< std::vector<cv::Point> > drawVec;
					drawVec.push_back(out.second);

					cv::Rect bounds = cv::boundingRect(out.second);
					last_good = out.second;

					cv::Scalar col(255,255,255);
					cv::drawContours(output, drawVec, 0, col);
					cv::putText(output, std::to_string(fps), cv::Point(50, 50), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 0, 255));
					cv::putText(output, std::to_string(getDistance(bounds.size(), src.size())) + " inches", cv::Point(50, 75), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 255, 0));
/*
					cv::putText(output, std::string("Horizontal ") + std::to_string(getFOVAngleHoriz(bounds.size(), src.size(), 36.0)) + " radians", cv::Point(50, 75), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 255, 0));
					cv::putText(output, std::string("Vertical ") + std::to_string(getFOVAngleVert(bounds.size(), src.size(), 36.0)) + " radians", cv::Point(50, 100), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(255, 0, 0));
*/
				
					cv::imshow("output", output);
				}

				if(cv::waitKey(30) >= 0) { 
/*
					cv::Rect bounds = cv::boundingRect(last_good);
					std::cout << "Horizontal: " << std::to_string(getFOVAngleHoriz(bounds.size(), src.size(), 36.0)) + " radians" << std::endl;
					std::cout << "Vertical: " << std::to_string(getFOVAngleVert(boundsize(), src.size(), 36.0)) + " radians" << std::endl;
*/
					break;
				}
			}
		}
}
#endif
