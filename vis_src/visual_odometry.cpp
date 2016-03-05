/*
 * Visual Odometry
 * Written by: Sebastian Mobo
 * 2/29/2016
 * An implementation of the monocular visual odometry system by Campbell, et al.
 * See: https://www.cs.cmu.edu/~personalrover/PER/ResearchersPapers/CampbellSukthankarNourbakhshPahwa_VisualOdometryCR.pdf
 *
 */

/* Algorithm Steps:
 * 1. Correct for lens distortions
 * 2. Find optical flow fields
 * 3. Filter unsmooth / unreliable flow vectors
 * 4. Project sky flow vectors onto robot-centered cylinder
 * 5. Filter vectors by consensus with compass data
 * 6. Calculate consensus incremental rotation
 * 7. Project ground flow vectors onto robot ground plane
 * 8. Unrotate ground flow vectors
 * 9. Filter vectors by consensus with accelerometer data
 */
#include "visual_odometry.h"

/* Debugging stuff. */
const std::string processWindowName = "processing";
const std::string posWindowName = "position";
const std::string skyTrackbarName = "Sky Bottom";
const std::string groundTrackbarName = "Ground Top";

const cv::Scalar	vectorColor = cv::Scalar(0, 0, 255);
const cv::Scalar	posColor = cv::Scalar(0, 255, 0);
const cv::Scalar	hdgColor = cv::Scalar(255, 0, 0);
const unsigned int 	hdgVectorLen	= 50;

const cv::Scalar	groundColor = cv::Scalar(0, 255, 0);
const cv::Scalar	skyColor = cv::Scalar(255, 0, 0);

const cv::Scalar	textColor = cv::Scalar(255, 255, 0);
const unsigned int	vectorDrawSz = 5;
const unsigned int	colorIncrement = 255 / nFramesBetweenCycles;

std::pair<double, double> projectToGroundPlane(cv::Point2f imgPoint) {
	double angleToGround = atan((2*double(imgPoint.y) - double(cameraSize.height)) * tan(cameraVFOV/2));
	//double angleX = atan((2*imgPoint.x - cameraSize.width) * tan(cameraHFOV/2));
	
	double y = cameraHeight / tan(angleToGround+cameraTiltAngle);
	
	double fovLenAtY = tan(cameraHFOV) / (2 * y);
	double x = fovLenAtY * ((double(imgPoint.x) - double(cameraSize.width / 2)) / double(cameraSize.width));
	
	return std::make_pair(x, y);
}

// assumes all sky points are at infinity
double projectToSkyCylinder(cv::Point imgPoint) {
	return (double((cameraSize.width / 2) - imgPoint.x) / double(cameraSize.width)) * cameraHFOV; // (cameraHFOV/ 2) * ((imgPoint.x - (cameraSize.width / 2)) / cameraSize.width);
}

unsigned int visOdo_state::getNumSkyVectors() {
	unsigned int n = 0;
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY < skyBottom) {
			n++;
		}
	}
	return n;
}

unsigned int visOdo_state::getNumGroundVectors() {
	unsigned int n = 0;
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY > groundTop) {
			n++;
		}
	}
	return n;
}

void visOdo_state::processSkyVectors() {
	for(visOdo_feature& i : this->trackpoints) {
		if((i.skyFlag > 0)|| i[0].frmY < skyBottom) {
			// project vectors to sky cylinder:
			i.skyFlag = 1;
			i[0].skyTheta = projectToSkyCylinder(i[0]);
			
			if(i.nVectors > 2) {
				double avgRotTheta = 0;
				for(int idx = 1; idx < i.nVectors; idx++) {
					avgRotTheta += (i[idx-1].skyTheta - i[idx].skyTheta);
				}

				avgRotTheta /= i.nVectors-1;
				i.rotTheta = avgRotTheta;
			}

			//std::cout << "sTheta[1] at x=" << i[1].frmX << " to " << (i[1].skyTheta * (180 / PI)) << std::endl;
			//std::cout << "sTheta[0] at x=" << i[0].frmX << " to " << (i[0].skyTheta * (180 / PI)) << std::endl;
			//std::cout << "nVectors = " << (unsigned short)i.nVectors << " rotTheta = " << i.rotTheta << std::endl;
		}
	}
}

void visOdo_state::processSkyVectors(double compassRot) {
	std::list<visOdo_feature> cTrackpoints;		// consistent
	std::list<visOdo_feature> icTrackpoints;	// inconsistent
	std::vector<visOdo_feature> newTrackpoints;
	
	for(visOdo_feature& i : this->trackpoints) {
		if((i.skyFlag > 0) || i[0].frmY < skyBottom) {
			// project vectors to sky cylinder:
			i.skyFlag = 1;
			i[0].skyTheta = projectToSkyCylinder(i[0]);
			
			if(i.nVectors >= 2) {
				// update apparent rotation:
				if(i.nVectors == 2) {
					i.rotTheta = (i[0].skyTheta - i[1].skyTheta) / 2;
				} else {
					i.rotTheta = (i.rotTheta + (i[0].skyTheta - i[1].skyTheta)) / 2;
				}
			
				// filter out "inconsistent" trackpoints (that don't match compass readings)
				if(abs(i.rotTheta - compassRot) > vecUnsmoothThres) {
					icTrackpoints.push_back(i);
				} else {
					cTrackpoints.push_back(i);	
				}
			} else {
				cTrackpoints.push_back(i);
			}
		}
	}
	
	for(visOdo_feature& i : cTrackpoints) {
		newTrackpoints.push_back(i);
	}
	
	if(cTrackpoints.size() > icTrackpoints.size()) {
		for(visOdo_feature& i : icTrackpoints) {
			i.score += vecQualityPenalty;
			if(i.score < vecQualityCutoff) {
				newTrackpoints.push_back(i);
			}
		}
	} else {
		for(visOdo_feature& i : icTrackpoints) {
			newTrackpoints.push_back(i);
		}
	}
	
	this->trackpoints = std::move(newTrackpoints);
}

void visOdo_state::processGroundVectors(double tX, double tY) {
	std::list<visOdo_feature> cTrackpoints;		// consistent
	std::list<visOdo_feature> icTrackpoints;	// inconsistent
	std::vector<visOdo_feature> newTrackpoints;
	
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY > groundTop) {
			// project vectors to ground plane:
			i.skyFlag = 0;
			std::pair<double, double> groundPlanePos = projectToGroundPlane(i[0]);
			
			i[0].groundX = groundPlanePos.first;
			i[0].groundY = groundPlanePos.second;
			
			if(i.nVectors >= 2) {
				// unrotate vectors:
				i[0].groundX = (i[0].groundX * cos(-this->last_rot)) - (i[0].groundY * sin(-this->last_rot));
				i[0].groundY = (i[0].groundX * sin(-this->last_rot)) + (i[0].groundY * cos(-this->last_rot));
			
				
				// update apparent translation:
				if(i.nVectors == 2) {
					i.trnsX = (i[0].groundX - i[1].groundX) / 2;
					i.trnsY = (i[0].groundY - i[1].groundY) / 2;
				} else {
					i.trnsX = (i.trnsX + (i[0].groundX - i[1].groundX)) / 2;
					i.trnsY = (i.trnsY + (i[0].groundY - i[1].groundY)) / 2;
				}
			
				// filter by accelerometer data:
				if((abs(i.trnsX - tX) > vecInconsistentTrans) ||
					(abs(i.trnsY - tY) > vecInconsistentTrans)) {
					icTrackpoints.push_back(i);
				} else {
					cTrackpoints.push_back(i);	
				}
			} else {
				cTrackpoints.push_back(i);
			}
		}
	}
	
	for(visOdo_feature& i : cTrackpoints) {
		newTrackpoints.push_back(i);
	}
	
	if(cTrackpoints.size() > icTrackpoints.size()) {
		for(visOdo_feature& i : icTrackpoints) {
			i.score += vecQualityPenalty;
			if(i.score < vecQualityCutoff) {
				newTrackpoints.push_back(i);
			}
		}
	} else {
		for(visOdo_feature& i : icTrackpoints) {
			newTrackpoints.push_back(i);
		}
	}
	
	this->trackpoints = std::move(newTrackpoints);
}

void visOdo_state::processGroundVectors() {
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY > groundTop) {
			// project vectors to ground plane:
			i.skyFlag = 0;
			std::pair<double, double> groundPlanePos = projectToGroundPlane(i[0]);
			
			i[0].groundX = groundPlanePos.first;
			i[0].groundY = groundPlanePos.second;
			
			if(i.nVectors >= 2) {
				// unrotate vectors:
				i[0].groundX = (i[0].groundX * cos(-this->last_rot)) - (i[0].groundY * sin(-this->last_rot));
				i[0].groundY = (i[0].groundX * sin(-this->last_rot)) + (i[0].groundY * cos(-this->last_rot));
			
				// update apparent translation:
				if(i.nVectors == 2) {
					i.trnsX = (i[0].groundX - i[1].groundX) / 2;
					i.trnsY = (i[0].groundY - i[1].groundY) / 2;
				} else {
					i.trnsX = (i.trnsX + (i[0].groundX - i[1].groundX)) / 2;
					i.trnsY = (i.trnsY + (i[0].groundY - i[1].groundY)) / 2;
				}
			}
			
		}
	}
}

std::vector<visOdo_feature> visOdo_state::getAllSkyFeatures() {
	std::vector<visOdo_feature> rVec;
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY < skyBottom) {
			rVec.push_back(i);
		}
	}
	
	return rVec;
}

std::vector<visOdo_feature> visOdo_state::getAllGroundFeatures() {
	std::vector<visOdo_feature> rVec;
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY > groundTop) {
			rVec.push_back(i);
		}
	}
	
	return rVec;
}

/*
 * Do filtering for unsmooth vectors and update quality scores.
 */
void visOdo_state::filterUnsmoothVectors() {
	std::list<visOdo_feature> smoothTrackpoints;
	std::list<visOdo_feature> unsmoothTrackpoints;
	std::vector<visOdo_feature> newTrackpoints;

	for(visOdo_feature& i : this->trackpoints) {
		// end - 1 = current frame, end - 2 = back 1 frame, etc...
		double dir73 = 0;
		double dir31 = 0;
		double dir1c = 0;
		
		// calculate vector direction values
		if(i.nVectors > 7) {
			double sumDispX = 0;
			double sumDispY = 0;
			for(unsigned int idx=3;idx<7;idx++) {
				sumDispX += (i[idx+1].frmX - i[idx].frmX);
				sumDispY += (i[idx+1].frmY - i[idx].frmY);
			}
			dir73 = atan2( sumDispY, sumDispX );
		}
		
		if(i.nVectors > 3) {
			double sumDispX = 0;
			double sumDispY = 0;
			for(unsigned int idx=1;idx<3;idx++) {
				sumDispX += (i[idx+1].frmX - i[idx].frmX);
				sumDispY += (i[idx+1].frmY - i[idx].frmY);
			}
			dir31 = atan2( sumDispY, sumDispX );
		}
		
		if(i.nVectors > 2) {
			visOdo_vector& p1 = i[1];
			visOdo_vector& pc = i[0];
			dir1c = atan2( p1.frmY - pc.frmY, p1.frmX - pc.frmX );
		}
		
		// check for unsmooth vectors
		bool smooth = true;
		if(i.nVectors > 7) {
			if(
			(abs(dir73 - dir31) > vecUnsmoothThres) ||
			(abs(dir73 - dir1c) > vecUnsmoothThres)
			) {
				smooth = false;
			}
		}
		
		if(i.nVectors > 3) {
			if(abs(dir31 - dir1c) > vecUnsmoothThres) {
				smooth = false;
			}
		}
		
		i.score = (i.score == 0 ? 0 : i.score - vecQualityDecay); //std::max(i.score - vecQualityDecay, (unsigned int)0);
		
		if(!smooth) {
			smoothTrackpoints.push_back(i);
		} else {
			unsmoothTrackpoints.push_back(i);
		}
	}
	
	for(visOdo_feature& i : smoothTrackpoints) {
		//std::cout << "smooth trackpoint at (" << i[0].frmX << ", " << i[0].frmY << ")" << std::endl;
		newTrackpoints.push_back(i);
	}
	
	std::cout << "filtered " << unsmoothTrackpoints.size() << " unsmooth vectors vs. " << smoothTrackpoints.size() << " smooth vectors." << std::endl;
	if(smoothTrackpoints.size() > unsmoothTrackpoints.size()) {
		// majority of elements are smooth, update scores
		for(visOdo_feature& i : unsmoothTrackpoints) {
			i.score += vecQualityPenalty;
			if(i.score < vecQualityCutoff) {
				newTrackpoints.push_back(i);
			}
		}
	} else {
		// don't update scores:
		for(visOdo_feature& i : unsmoothTrackpoints) {
			newTrackpoints.push_back(i);
		}
	}
	
	this->trackpoints = std::move(newTrackpoints);
}

void visOdo_state::findOpticalFlow(cv::Mat nextFrame) {
	std::vector<cv::Point2f> newPoints(this->lastPoints.size());
	std::vector<unsigned char> status(this->lastPoints.size());
	std::vector<float> err(this->lastPoints.size());
	
	// find new points
	//std::cout << "Number points: " << this->lastPoints.size() << std::endl;

	cv::calcOpticalFlowPyrLK(this->lastFrame,
		nextFrame,
		this->lastPoints,
		newPoints,
		status,
		err,
		cv::Size(31, 31),
		0,
		cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 30, 0.01),
		0,
		1e-4
		);
	
	// match old points to new points
	unsigned short idx = 0;
	std::vector<visOdo_feature> newTrackpoints;
	for(cv::Point2f& i : this->lastPoints) {
		if((i.x == 0 && i.y == 0) || i.x > cameraSize.width || i.y > cameraSize.height) {
			idx++;
			continue;
		}

		if(status[idx] == 1) {	
			for(visOdo_feature& i2 : this->trackpoints) {
				if(i2[0].id == idx) {
					//std::cout << "matched feature at: (" << i.x << ", " << i.y << ")" << std::endl;
					addFeatureElement(i2, newPoints[idx], idx);
					newTrackpoints.push_back(i2);
				}
			}
		}
		idx++;
	}
	
	this->trackpoints = newTrackpoints;
	this->lastPoints = newPoints;
	this->lastFrame = nextFrame;
	this->ttl--;
}

bool compareFeatureRotation(const visOdo_feature& a, const visOdo_feature& b) {
	return (a.rotTheta < b.rotTheta);
}

bool compareFeatureTranslation(const visOdo_feature& a, const visOdo_feature& b) {
	return (a.trnsX + a.trnsY) < (b.trnsX + b.trnsY);
}

void visOdo_state::findConsensusRotation() {
	std::vector<visOdo_feature> skyVectors = visOdo_state::getAllSkyFeatures();
	/*
	double avgRot = 0;
	unsigned int nAveraged = 0;	

	for(visOdo_feature& i : skyVectors) {
		//std::cout << "nVectors: " << (unsigned short)i.nVectors << ", rotTheta = " << i.rotTheta << std::endl;
				
		if(!std::isnan(i.rotTheta)) {	
			avgRot += i.rotTheta;
			nAveraged++;
		} else {
			std::cout << "error: i.rotTheta == nan" << std::endl;
			for(int idx = 0; idx < i.nVectors; idx++) {
				std::cout << "sTheta[" << idx << "] at x=" << i[idx].frmX << " to " << (i[idx].skyTheta * (180 / PI)) << std::endl;
			}
		}
	}

	std::cout << "averaging " << nAveraged << " sky vectors." << std::endl;

	avgRot /= nAveraged;

	//std::cout << "Mean rotation: " << avgRot * (180 / PI) << std::endl;

	this->last_rot = avgRot;
	*/


	
	std::array<unsigned int, rotHistogramBinNum> bins;
	
	std::sort(skyVectors.begin(), skyVectors.end(), compareFeatureRotation);
	
	//std::cout << "found " << skyVectors.size() << " sky vectors." << std::endl;

	double histogramRange = skyVectors.back().rotTheta - skyVectors.front().rotTheta;
	double histogramBinSz = histogramRange / double(rotHistogramBinNum);
	double startRotTheta = skyVectors.front().rotTheta;
	
	for(unsigned int i=0;i<rotHistogramBinNum;i++) {
		bins[i] = 0;
	}

	for(visOdo_feature& i : skyVectors) {
		unsigned int binNum = ((i.rotTheta - startRotTheta) / histogramBinSz);
		
		bins[binNum]++;
	}
	
	double rotWeightedAvg = 0;
	
	unsigned int idx = 0;
	double currentRotTheta = startRotTheta;
	for(unsigned int i : bins) {
		double weight = (double(i) / double(skyVectors.size()));
		
		rotWeightedAvg += (currentRotTheta * weight);
		
		idx++;
		currentRotTheta += histogramBinSz;
	}
	
	this->last_rot = rotWeightedAvg;
	
}

void visOdo_state::findConsensusTranslation() {
	// get 40 random elements, find one with biggest following
	
	/*
	std::vector<unsigned int> elements;
	
	for(unsigned int i=0;i<skyVectors.size();i++) {
		elements.push_back(i);
	};
	
	std::array<visOdo_vector, consensusRandGroupSz> binDef;
	std::array<unsigned int, consensusRandGroupSz> bins;
		
	std::shuffle(elements.begin(), elements.end(), std::mt19937());
	
	for(unsigned int i=0;i<consensusRandGroupSz;i++) {
		binDef[i] = skyVectors[elements[i]].rotTheta;
	}
	
	
	*/
	
	// get weighted mean of all translation vectors
	
	double sumX = 0;
	double sumY = 0;
	
	std::vector<visOdo_feature> groundVectors = this->getAllGroundFeatures();
	
	for(visOdo_feature& i : groundVectors) {
		// we flip the signs of the Y-coordinates of all vectors here.
		// OpenCV has the origin at the top-left hand corner-- positive translations correspond to points moving down and right in the image plane.
		// since we see all translations as "reversed" (when we move left, the points in the image move right, etc.), however, all directions are flipped:
		
		// Where X is left-right (perpendicular to heading) and Y is forward-backwards (parallel to heading):
		// +X = left 
		// -X = right
		// +Y = forward
		// +Y = back
		
		// Positive translations in the robot plane, however, need to be up and right, however.
		
		sumX -= i.trnsX;
		sumY += i.trnsY;
	}
	
	this->last_transX = sumX / groundVectors.size();
	this->last_transY = sumY / groundVectors.size();
}

void visOdo_state::accumulateMovement() {
	auto now = std::chrono::steady_clock::now();
	
	std::chrono::duration<double> diff = (now - this->lastTS); // in seconds
	
	this->hdg += this->last_rot; //(this->last_rot * diff.count());
	this->posX += this->last_transX; //((this->last_transX * cos(this->hdg)) * diff.count());
	this->posY += this->last_transY; //(this->last_transY * sin(this->hdg) * diff.count());
	
	
	this->lastTS = now;
}

void visOdo_state::startCycle(cv::Mat frame) {
	std::vector<cv::Point2f> newFeatures;
	cv::goodFeaturesToTrack(frame, newFeatures, nFeaturesTracked, minFeatureQuality, minDistFeatures);
	
	this->ttl = nFramesBetweenCycles;
	
	std::vector<visOdo_feature> newTrackpoints;
	unsigned  idx = 0;
	for(cv::Point2f i : newFeatures) {
		// discard invalid vectors
		if((i.x == 0 && i.y == 0) || i.x > cameraSize.width || i.y > cameraSize.height) {
			continue;
		}

		//std::cout << "feature at: (" << i.x << ", " << i.y << ")" << std::endl;
		newTrackpoints.emplace(newTrackpoints.end(), i, idx);
		idx++;
	}
	
	this->trackpoints = newTrackpoints;
	this->lastPoints = newFeatures;
	this->lastFrame = frame;
	this->lastTS = std::chrono::steady_clock::now();
}

void visOdo_state::doCycle(cv::Mat frame, double compassRot, double tX, double tY) {
		if((this->ttl <= 0) || (this->trackpoints.size() < minNumFeatures)) {
			this->startCycle(frame);
		} else {
			this->findOpticalFlow(frame);
			if(this->trackpoints.size() < minNumFeatures) {
				std::cout << "could not track vectors" << std::endl;
				return;
			} else {
				std::cout << "tracking " << this->trackpoints.size() << " features" << std::endl;
			}
			this->filterUnsmoothVectors();
			this->processSkyVectors(compassRot);
			this->findConsensusRotation();
			this->processGroundVectors(tX, tY);
			this->findConsensusTranslation();
			this->accumulateMovement();
		}
		
		this->ttl--;
}

void visOdo_state::doCycle(cv::Mat frame) {
		if(this->ttl <= 0 || (this->trackpoints.size() < minNumFeatures)) {
			this->startCycle(frame);
			this->processSkyVectors();
			this->processGroundVectors();
		} else {
			this->findOpticalFlow(frame);
			if(this->trackpoints.size() < minNumFeatures) {
				std::cout << "could not track vectors" << std::endl;
				return;
			} else {
				std::cout << "tracking " << this->trackpoints.size() << " features" << std::endl;
			}
			this->filterUnsmoothVectors();

			unsigned int nGround = 0;
			unsigned int nSky = 0;

			if((nSky = this->getNumSkyVectors()) == 0) {
				std::cout << "could not find any sky vectors" << std::endl;
				return;
			} else {
				std::cout << "tracking " << nSky << " sky features" << std::endl;
			}

			if((nGround = this->getNumGroundVectors()) == 0) {
				std::cout << "could not find any ground vectors" << std::endl;
				return;
			} else {
				std::cout << "tracking " << nGround << " ground features" << std::endl;
			}

			this->processSkyVectors();
			this->findConsensusRotation();
			this->processGroundVectors();
			this->findConsensusTranslation();
			this->accumulateMovement();
		}
		
		this->ttl--;
}

void onTrackbarUpdate(int pos, void* ptr) {
	if(groundTop < skyBottom) {
		groundTop = skyBottom+1;
	}
}


int main() {
	cv::namedWindow(processWindowName);
	cv::namedWindow(posWindowName);

	visOdo_state odoSt;
	
	cv::Mat posOutputWindow(cv::Size(800, 800), CV_8UC3);
	
	cv::VideoCapture cam(0);
	
	if(!cam.isOpened())
		return -1;

	cameraSize = cv::Size(cam.get(CV_CAP_PROP_FRAME_WIDTH), cam.get(CV_CAP_PROP_FRAME_HEIGHT));

	cv::Mat currentVectorPos(cameraSize, CV_8UC3);

	cv::createTrackbar(groundTrackbarName, processWindowName, &groundTop, cameraSize.height, onTrackbarUpdate);
	cv::createTrackbar(skyTrackbarName, processWindowName, &skyBottom, cameraSize.height, onTrackbarUpdate);
	
	cam.set(CV_CAP_PROP_FPS, 15.0);

	std::chrono::steady_clock::time_point lst = std::chrono::steady_clock::now();
	double fpsSum = 0;
	unsigned int nFrames = 0;
	double t = (double)cv::getTickCount();

	while(true) {
		cv::Mat img;
		cam >> img;

		cv::Mat blurOut;
		//cv::bilateralFilter(img, blurOut, 9, 9*2, 9/2);
		cv::GaussianBlur(img, blurOut, cv::Size(9,9), 0, 0);

		//if(odoSt.ttl == 0) {
			currentVectorPos = cv::Mat::zeros(img.size(), img.type());
			posOutputWindow = cv::Mat::zeros(cv::Size(800, 800), CV_8UC3);
		//}
		
		

		cv::Mat inImg(blurOut.size(), CV_8U);		
		cv::cvtColor(blurOut, inImg, CV_BGR2GRAY, 1);

		odoSt.doCycle(inImg);

		cv::Mat copy = blurOut; //inImg.clone();
		
		for(visOdo_feature& i : odoSt.trackpoints) {
				cv::Point pt = i[0];
				//std::cout << "feature at: (" << pt.x << ", " << pt.y << ")" << std::endl;				
				//currentVectorPos.at<cv::Scalar>(pt) = vectorColor;
				
				cv::circle(copy, pt, vectorDrawSz, vectorColor, -1);
		}
		
		//currentVectorPos.copyTo(copy, currentVectorPos);
		
		//std::cout << "ttl: " << odoSt.ttl << std::endl;
		//std::cout << "Number points: " << odoSt.lastPoints.size() << std::endl;

		//if(odoSt.ttl == 0) {
			unsigned int posXft = (unsigned int)(odoSt.posX * 3.28084);
			unsigned int posYft = (unsigned int)(odoSt.posY * 3.28084);

			double compassNeedleX = hdgVectorLen * cos(odoSt.hdg + (PI / 2));
			double compassNeedleY = hdgVectorLen * sin(odoSt.hdg + (PI / 2));
			
			posOutputWindow.at<cv::Scalar>(cv::Point(posXft+400, posYft+400)) = posColor;
			cv::line(posOutputWindow, cv::Point(400, 400), cv::Point(400+compassNeedleX, 400+compassNeedleY), hdgColor);
		//}
		
		double fps = 1 / (((double)cv::getTickCount() - t) / cv::getTickFrequency());
		fpsSum += fps;
		nFrames++;
		t = (double)cv::getTickCount();

		cv::line(copy, cv::Point(0, groundTop), cv::Point(cameraSize.width-1, groundTop), groundColor);
		cv::line(copy, cv::Point(0, skyBottom), cv::Point(cameraSize.width-1, skyBottom), skyColor);

		cv::putText(copy, "current fps: " + std::to_string(fpsSum / nFrames) + " (" + std::to_string(fps) + ")", cv::Point(50, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "current heading: " + std::to_string(odoSt.hdg * (180 / PI)), cv::Point(50, 50),cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "number features: " + std::to_string(odoSt.trackpoints.size()), cv::Point(50, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		
		cv::putText(copy, "velX:  " + std::to_string(odoSt.last_transX), cv::Point(50, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "velY:  " + std::to_string(odoSt.last_transY), cv::Point(50, 150), cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "velRot: " + std::to_string(odoSt.last_rot * (180 / PI)), cv::Point(50, 175), cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		
		cv::imshow(processWindowName, copy);
		cv::imshow(posWindowName, posOutputWindow);
		
		if(cv::waitKey(30) > 0) {
			break;
		}
	}
}
