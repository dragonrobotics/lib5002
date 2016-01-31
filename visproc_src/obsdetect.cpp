#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

typedef std::vector<cv::Point> contour;
typedef unsigned int** hv_histogram;

contour createTrapezoid(unsigned int topWidth, unsigned int baseWidth, unsigned int height, cv::Point bottomLeft) {
	contour out;
	out.push_back(bottomLeft);
	out.push_back(cv::Point(bottomLeft.x + baseWidth, bottomLeft.y));
	out.push_back(cv::Point(bottomLeft.x + topWidth + ((baseWidth - topWidth)/2), bottomLeft.y-height));
	out.push_back(cv::Point(bottomLeft.x + ((baseWidth - topWidth)/2), bottomLeft.y-height));	
	return out;
}

const contour refTrpz = createTrapezoid(160, 480, 120, cv::Point(80, 470));
const unsigned int valThres = 80;
const unsigned int satThres = 60;

int valHistBinThres = 370;
int satHistBinThres = 325;
int hueHistBinThres = 300;

hv_histogram getReferenceHistograms(cv::Mat src) {
	cv::Mat refTrap(src.size(), src.type());
	cv::Mat mask(src.size(), CV_8U);

	cv::fillConvexPoly(mask, refTrpz.data(), 4, 255);	
	src.copyTo(refTrap, mask);

	hv_histogram hists = new unsigned int*[3];
	hists[0] = new unsigned int[256];
	hists[1] = new unsigned int[256];
	hists[2] = new unsigned int[256];

	unsigned int* hueHist = hists[0];
	unsigned int* satHist = hists[1];
	unsigned int* valHist = hists[2];

	for(unsigned int i=0;i<256;i++) {
		hueHist[i] = 0;
		satHist[i] = 0;
		valHist[i] = 0;
	}

	for(cv::MatIterator_<cv::Vec3b> it = refTrap.begin<cv::Vec3b>(); it != refTrap.end<cv::Vec3b>(); ++it) {
		if((*it)[2] > 0) {		
			valHist[(*it)[2]]++;
			if((*it)[2] >= valThres) {
				satHist[(*it)[1]]++;				
				if((*it)[1] >= satThres) {
					hueHist[(*it)[0]]++;
				}
			}
		}
	}

	return hists;
}

cv::Mat obs_getObsMask(cv::Mat src) {
	cv::Mat out = cv::Mat::zeros(src.size(), CV_8U);
	cv::Mat tmp(src.size(), src.type());

	cv::GaussianBlur(src, tmp, cv::Size(5,5), 2.5, 2.5, cv::BORDER_DEFAULT);
	cv::cvtColor(tmp, tmp, CV_BGR2HSV);
	
	hv_histogram refHist = getReferenceHistograms(tmp);

	for(cv::MatIterator_<cv::Vec3b> it = tmp.begin<cv::Vec3b>(); it != tmp.end<cv::Vec3b>(); ++it) {
		unsigned char& p = out.at<unsigned char>(it.pos());

		if(refHist[2][(*it)[2]] < valHistBinThres) {
			p = 255;
			continue;		
		}

		if((*it)[2] >= valThres) {
			if(refHist[1][(*it)[1]] < satHistBinThres) {
				p = 255;
				continue;			
			}
			if((*it)[1] >= satThres) {
				if(refHist[0][(*it)[0]] < hueHistBinThres) {
					p = 255;				
				}
			}
		}
	}

	return out;
}

int main(int argc, char** argv) {
	cv::VideoCapture cap(1); // open cam 1
	if(!cap.isOpened())  // check if we succeeded
		return -1;

	cv::namedWindow("input");
	cv::namedWindow("mask");
	cv::namedWindow("output");

	std::cout << "Width: " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << std::endl;
	std::cout << "Height: " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl;

	cvCreateTrackbar("Val Thres", "input", &valHistBinThres, 500, NULL);
	cvCreateTrackbar("Sat Thres", "input", &satHistBinThres, 500, NULL);
	cvCreateTrackbar("Hue Thres", "input", &hueHistBinThres, 500, NULL);

	while(true) {
		cv::Mat src;
		cap >> src;

		std::vector<contour> drawVec;
		drawVec.push_back(refTrpz);

		cv::Mat tmp;
		src.copyTo(tmp);
		cv::drawContours(tmp, drawVec, -1, cv::Scalar(255,255,255));

		cv::imshow("input", tmp);
	
		cv::Mat obsMask = obs_getObsMask(src);

		cv::imshow("mask", obsMask);

		cv::Mat edgedet;

		cv::blur(obsMask, edgedet, cv::Size(3,3));

		cv::Mat detOut;		
		cv::Canny(edgedet, detOut, 10, 20);		

		cv::imshow("output", detOut);

		if(cv::waitKey(30) > 0) break;
	}

	return 0;
}
