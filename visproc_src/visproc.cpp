#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

/* Thresholds for goal detection: */
/* hue threshold: min/max; target seems to be 89 for the test image. */
/* 75-100 (150 - 200 with 360 degree Hue) */
const unsigned char hueThres[2] = {85, 95};
/* value threshold as above. target seems to be 59. */
/* though of course you need to make it 0-255 instead of 0-100 */
/* 45 - 75 works? though it's 115 - 191 in OpenCV */
const unsigned char valThres[2] = {115, 191};

const double cannyThresMin = 10;
const double cannyThresSize = 10; /* Max = ThresMin + ThresSize */
const double pi = 3.14159265358979323846;

void convert_and_write(const char* filename, cv::Mat hsvimg) {
        cv::Mat tmp;
        cv::cvtColor(hsvimg, tmp, CV_HSV2BGR);
        cv::imwrite(filename, tmp);
}

cv::Mat goal_preprocess_pipeline(cv::Mat input) {
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
        cv::imwrite("pipeline_stage4.png", edgedet);

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
        cv::imwrite("pipeline_stage5.png", edgedet);

        return edgedet;
}

cv::Mat boulder_preprocess_pipeline(cv::Mat input) {
        cv::Mat tmp(input.size(), input.type());

        cv::cvtColor(input, tmp, CV_BGR2HSV);

        /* Make things easier for the HV filter */
        //cv::blur(tmp, tmp, cv::Size(5,5));
        cv::GaussianBlur(tmp, tmp, cv::Size(7,7), 3.5, 3.5, cv::BORDER_REPLICATE);
        convert_and_write("pipeline_stage1.png", tmp);

        /* Filter on saturation and brightness */
        cv::Mat mask(input.size(), CV_8U);
        cv::inRange(tmp, cv::Scalar(0,0,102), cv::Scalar(255,26,191), mask);
        cv::imwrite("pipeline_stage2.png", mask);

        /* Erode away smaller hits */
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7)));
        cv::imwrite("pipeline_stage3.png", mask);

        /* Blur for edge detection */
        cv::Mat edgedet;
        cv::blur(mask, edgedet, cv::Size(5,5));
        cv::imwrite("pipeline_stage4.png", edgedet);

        cv::Canny(edgedet, edgedet, cannyThresMin, cannyThresMin+cannyThresSize);
        cv::imwrite("pipeline_stage5.png", edgedet);

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
                const double area_threshold = 1000;

                std::cout << std::endl;
                std::cout << "Contour " << ctr << ": " << std::endl;
                ctr++;
                std::cout << "Area: "  << area << std::endl;
                std::cout << "Perimeter: " << perimeter << std::endl;

                /* Area Thresholding Test
                 * Only accept contours of a certain total size.
                 */

                if(area < area_threshold) {
                    continue;
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

                std::cout << "nu-02: " << m.nu02 << std::endl;
                std::cout << "CVArea: "  <<  coverage_area << std::endl;
                std::cout << "AsRatio: " << aspect_ratio << std::endl;
                std::cout << "Orientation: " << theta << std::endl;

                double total_score = (moment_score + cvarea_score + ar_score + angle_score) / 4;

                std::cout << "CVArea Score: "  <<  cvarea_score << std::endl;
                std::cout << "AsRatio Score: " << ar_score << std::endl;
                std::cout << "Moment Score: " << moment_score << std::endl;
                std::cout << "Angle Score: " << angle_score << std::endl;
                std::cout << "Total Score: " << total_score << std::endl;

                finalscores.push_back(std::make_pair(total_score, std::move(*i)));
        }

        std::sort(finalscores.begin(), finalscores.end(), &scoresort);

        return finalscores.back();
}

scoredContour boulder_pipeline(cv::Mat input) {
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

            const double area_threshold = 5000;

            if(area < area_threshold) {
                continue;
            }

            std::cout << std::endl;
            std::cout << "Contour " << ctr << ": " << std::endl;
            ctr++;
            std::cout << "Area: "  << area << std::endl;
            std::cout << "Perimeter: " << perimeter << std::endl;

            double idealRadius = (bounds.width/2);
            double idealArea = pi * (idealRadius * idealRadius);

            double circularity = scoreDistanceFromTarget(idealArea, area);
            double ar_score = scoreDistanceFromTarget(1, bounds.width / bounds.height);

            double total_score = (circularity + ar_score) / 2;

            std::cout << "Circularity Score: " << circularity << std::endl;
            std::cout << "AsRatio Score: " << ar_score << std::endl;
            std::cout << "Total Score: " << total_score << std::endl;

            finalscores.push_back(std::make_pair(total_score, std::move(*i)));
    }

    std::sort(finalscores.begin(), finalscores.end(), &scoresort);

    return finalscores.back();
}

int main(int argc, char** argv) {
        if(argc == 1) {
            std::cout << "Need input file to process." << std::endl;
            return 1;
        }
        std::string infile = argv[1];
        bool boulderproc = false;
        if(argc > 2) {
            std::string opt2 = argv[2];
            if(opt2 == "--boulder") {
                boulderproc = true;
            }
        }

        cv::Mat src = cv::imread(infile, 1);

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
}
