#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

/* hue threshold: min/max; target seems to be 89 for the test image. */
/* 75-100 (150 - 200 with 360 degree Hue) */
const unsigned char hueThres[2] = {85, 95};
/* value threshold as above. target seems to be 59. */
/* though of course you need to make it 0-255 instead of 0-100 */
/* 45 - 75 works? though it's 115 - 191 in OpenCV */
const unsigned char valThres[2] = {115, 191};
const double cannyThresMin = 10;
const double cannyThresSize = 10; /* Max = ThresMin + ThresSize */

void convert_and_write(const char* filename, cv::Mat hsvimg) {
    cv::Mat tmp;
    cv::cvtColor(hsvimg, tmp, CV_HSV2BGR);
    cv::imwrite(filename, tmp);
}

cv::Mat preprocess_pipeline(cv::Mat input) {
        cv::Mat tmp(input.size(), input.type());

        cv::cvtColor(input, tmp, CV_BGR2HSV);

        /* Make things easier for the HV filter */
        //cv::blur(tmp, tmp, cv::Size(5,5));
        cv::GaussianBlur(tmp, tmp, cv::Size(5,5), 2.5, 2.5, cv::BORDER_REPLICATE);
        convert_and_write("pipeline_stage1.png", tmp);

        /* Filter on color/brightness */
        cv::Mat mask(input.size(), CV_8U);
        cv::inRange(tmp, cv::Scalar(hueThres[0],0,valThres[0]), cv::Scalar(hueThres[1],255,valThres[1]), mask);
        cv::imwrite("pipeline_stage2.png", mask);

        /* Erode away smaller hits */
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5)));
        cv::imwrite("pipeline_stage3.png", mask);

        /* Blur for edge detection */
        cv::Mat edgedet;
        cv::blur(mask, edgedet, cv::Size(3,3));
        cv::imwrite("pipeline_stage4.png", mask);

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
        cv::imwrite("pipeline_stage5.png", mask);

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

scoredContour goal_pipeline(cv::Mat input) {
        std::vector< std::vector<cv::Point> > contours;

        cv::Mat contourOut = input.clone();

        cv::findContours(contourOut, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        std::vector<scoredContour> finalscores;

        std::cout << "Found " << contours.size() << " contours." << std::endl;

        unsigned int ctr = 0;
        for(std::vector< std::vector<cv::Point> >::iterator i = contours.begin();
            i != contours.end(); ++i) {
                double area = cv::contourArea(*i);
                double perimeter = cv::arcLength(*i, true);
                cv::Rect bounds = cv::boundingRect(*i);

                const double cvarea_target = (80.0/240.0);
                const double asratio_target = (20.0/12.0);

                std::cout << std::endl;
                std::cout << "Contour " << ctr << ": " << std::endl;
                std::cout << "Area: "  << area << std::endl;
                std::cout << "Perimeter: " << perimeter << std::endl;

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

                std::cout << "CVArea: "  <<  coverage_area << std::endl;
                std::cout << "AsRatio: " << aspect_ratio << std::endl;

                double total_score = (cvarea_score + ar_score) / 2;

                total_score += area; // prioritize larger blobs

                std::cout << "CVArea Score: "  <<  cvarea_score << std::endl;
                std::cout << "AsRatio Score: " << ar_score << std::endl;
                std::cout << "Total Score: " << total_score << std::endl;

                ctr++;
                finalscores.push_back(std::make_pair(total_score, std::move(*i)));
        }

        std::sort(finalscores.begin(), finalscores.end(), &scoresort);

        return finalscores.back();
}

int main(int argc, char** argv) {
        std::string infile = argv[1];

        cv::Mat src = cv::imread(infile, 1);

        scoredContour out = goal_pipeline(preprocess_pipeline(src));

        cv::Mat output = cv::Mat::zeros(src.size(), CV_8UC3);

        std::vector< std::vector<cv::Point> > drawVec;
        drawVec.push_back(out.second);

        cv::Scalar col(255,255,255);
        cv::drawContours(output, drawVec, 0, col);

        cv::imwrite("pipeline_output.png", output);
}
