#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

/* Thresholds for goal detection: */
/* hue threshold: min/max; target seems to be 89 for the test image. */
/* 75-100 (150 - 200 with 360 degree Hue) */
int hueThres[2] = {70, 100};
/* value threshold as above. target seems to be 59. */
/* though of course you need to make it 0-255 instead of 0-100 */
/* 45 - 75 works? though it's 115 - 191 in OpenCV */
int valThres[2] = {128, 255};

const double cannyThresMin = 10;
const double cannyThresSize = 10; /* Max = ThresMin + ThresSize */
const double pi = 3.14159265358979323846;

void convert_and_write(const char* filename, cv::Mat hsvimg) {
        cv::Mat tmp;
        cv::cvtColor(hsvimg, tmp, CV_HSV2BGR);
        cv::imwrite(filename, tmp);
}

cv::Mat goal_preprocess_pipeline(cv::Mat input, bool suppress_output=false, bool live_output=false) {
        cv::Mat tmp(input.size(), input.type());

        cv::cvtColor(input, tmp, CV_BGR2HSV);

        /* Make things easier for the HV filter */
        //cv::blur(tmp, tmp, cv::Size(5,5));
        cv::GaussianBlur(tmp, tmp, cv::Size(5,5), 2.5, 2.5, cv::BORDER_REPLICATE);
        if(!suppress_output) { convert_and_write("pipeline_stage1.png", tmp); }
		if(live_output) { cv::imshow("stage1", tmp); }

        /* Filter on color/brightness */
        cv::Mat mask(input.size(), CV_8U);
        cv::inRange(tmp, cv::Scalar((unsigned char)hueThres[0],0,(unsigned char)valThres[0]), cv::Scalar((unsigned char)hueThres[1],255,(unsigned char)valThres[1]), mask);
        if(!suppress_output) { cv::imwrite("pipeline_stage2.png", mask); }
		if(live_output) { cv::imshow("stage2", mask); }

        /* Erode away smaller hits */
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5)));
        if(!suppress_output) { cv::imwrite("pipeline_stage3.png", mask); }
		if(live_output) { cv::imshow("stage3", mask); }

        /* Blur for edge detection */
        cv::Mat edgedet;
        cv::blur(mask, edgedet, cv::Size(3,3));
        if(!suppress_output) { cv::imwrite("pipeline_stage4.png", edgedet); }
		if(live_output) { cv::imshow("stage4", edgedet); }

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
        if(!suppress_output) { cv::imwrite("pipeline_stage5.png", edgedet); }
		if(live_output) { cv::imshow("stage5", edgedet); }

        return edgedet;
}

cv::Mat boulder_preprocess_pipeline(cv::Mat input, bool suppress_output=false, bool live_output=false) {
        cv::Mat tmp(input.size(), input.type());

        cv::cvtColor(input, tmp, CV_BGR2HSV);

        /* Make things easier for the HV filter */
        //cv::blur(tmp, tmp, cv::Size(5,5));
        cv::GaussianBlur(tmp, tmp, cv::Size(5,5), 2.5, 2.5, cv::BORDER_DEFAULT);
        if(!suppress_output) {convert_and_write("pipeline_stage1.png", tmp); }
		if(live_output) { cv::imshow("stage1", tmp); }

        /* Filter on saturation and brightness */
        cv::Mat mask(input.size(), CV_8U);
        cv::inRange(tmp, cv::Scalar(0,0,90), cv::Scalar(180,35,255), mask);
        if(!suppress_output) { cv::imwrite("pipeline_stage2.png", mask); }
		if(live_output) { cv::imshow("stage2", mask); }

        /* Erode away smaller hits */
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7)));
        if(!suppress_output) { cv::imwrite("pipeline_stage3.png", mask); }
		if(live_output) { cv::imshow("stage3", mask); }

        /* Blur for edge detection */
        cv::Mat edgedet;
        cv::blur(mask, edgedet, cv::Size(9,9));
        if(!suppress_output) { cv::imwrite("pipeline_stage4.png", edgedet); }
		if(live_output) { cv::imshow("stage4", edgedet); }

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
        if(!suppress_output) { cv::imwrite("pipeline_stage5.png", edgedet); }
		if(live_output) { cv::imshow("stage5", edgedet); }

        return edgedet;
}

typedef std::pair< double, std::vector<cv::Point> > scoredContour;

bool scoresort(scoredContour c1, scoredContour c2) {
        return (c1.first < c2.first);
}

double scoreDistanceFromTarget(const double target, double value) {
        double distanceRatio = (fabs(target - fabs(target - value)) / target);
        return fmax(0, fmin(distanceRatio*100, 100));
}

scoredContour goal_pipeline(cv::Mat input, bool suppress_output=false) {
        std::vector< std::vector<cv::Point> > contours;

        cv::Mat contourOut = input.clone();

        cv::findContours(contourOut, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        std::vector<scoredContour> finalscores;

        if(!suppress_output) { std::cout << "Found " << contours.size() << " contours." << std::endl; }

        unsigned int ctr = 0;
        for(std::vector< std::vector<cv::Point> >::iterator i = contours.begin();
            i != contours.end(); ++i) {
                double area = cv::contourArea(*i);
                double perimeter = cv::arcLength(*i, true);
                cv::Rect bounds = cv::boundingRect(*i);

                const double cvarea_target = (80.0/240.0);
                const double asratio_target = (20.0/12.0);
                const double area_threshold = 1000;

                /* Area Thresholding Test
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

                /* Coverage Area Test
                 * Compare particle area vs. Bounding Rectangle area.
                 * score = 1 / abs((1/3)- (particle_area / boundrect_area))
                 * Score decreases linearly as coverage area tends away from 1/3. */
                double cvarea_score = 0;

                double coverage_area = area / bounds.area();
                cvarea_score = scoreDistanceFromTarget(cvarea_target, coverage_area);

                /* Aspect Ratio Test
                 * Computes aspect ratio of detected objects.
                 */

                double tmp = bounds.width;
                double aspect_ratio = tmp / bounds.height;
                double ar_score = scoreDistanceFromTarget(asratio_target, aspect_ratio);

                /* Image Moment Test
                 * Computes image moments and compares it to known values.
                 */

                cv::Moments m = cv::moments(*i);
                double moment_score = scoreDistanceFromTarget(0.28, m.nu02);

                /* Image Orientation Test
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

scoredContour boulder_pipeline(cv::Mat input, bool suppress_output=false, bool window_output=false) {
    std::vector< std::vector<cv::Point> > contours;

    cv::Mat contourOut = input.clone();

    cv::findContours(contourOut, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    std::vector<scoredContour> finalscores;

    std::cout << "Found " << contours.size() << " contours." << std::endl;

    unsigned int ctr = 0;
    for(std::vector< std::vector<cv::Point> >::iterator i = contours.begin();
        i != contours.end(); ++i) {
            ctr++;
            double area = cv::contourArea(*i);
            double perimeter = cv::arcLength(*i, true);
            cv::Rect bounds = cv::boundingRect(*i);

            const double area_threshold = 500;

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

    	return finalscores.back();
	} else {
		return std::make_pair(0.0, std::vector<cv::Point>());	
	}
}

void val_min_adjust(int v, void* ignore) { valThres[0] = v; }
void val_max_adjust(int v, void* ignore) { valThres[1] = v; }

void hue_min_adjust(int v, void* ignore) { hueThres[0] = v; }
void hue_max_adjust(int v, void* ignore) { hueThres[1] = v; }

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

					cv::Scalar col(255,255,255);
					cv::drawContours(output, drawVec, 0, col);
					cv::putText(output, std::to_string(fps), cv::Point(50, 50), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar(0, 0, 255));
				
					cv::imshow("output", output);
				}

				if(cv::waitKey(30) >= 0) break;
			}
		}
}
