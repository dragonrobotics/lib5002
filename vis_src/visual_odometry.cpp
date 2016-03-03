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
 * 10. Calculate consensus incremental translation
 */
#include "visual_odometry.h"

/* Debugging stuff. */
const std::string processWindowName = "processing";
const std::string posWindowName = "position";
const std::string skyTrackbarName = "Sky Bottom";
const std::string groundTrackbarName = "Ground Top";

const cv::Scalar	vectorColor = cv::Scalar(0, 0, 255);
const cv::Scalar	posColor = cv::Scalar(, 0, 255);

const cv::Scalar	groundColor = cv::Scalar(0, 255, 0);
const cv::Scalar	skyColor = cv::Scalar(255, 0, 0);

const cv::Scalar	textColor = cv::Scalar(255, 255, 0);
const unsigned int	vectorDrawSz = 3;
const unsigned int	colorIncrement = 255 / nFramesBetweenCycles;

inline std::pair<double, double> projectToGroundPlane(cv::Point imgPoint) {
	double angleToGround = atan((2*imgPoint.y - cameraSize.height) * tan(cameraVFOV/2));
	//double angleX = atan((2*imgPoint.x - cameraSize.width) * tan(cameraHFOV/2));
	
	double y = height / tan(angleToGround+cameraTiltAngle);
	
	double fovLenAtY = tan(cameraHFOV) / (2 * y);
	double x = fovLenAtY * ((imgPoint.x - (cameraSize.width / 2)) / cameraSize.width);
	
	return std::make_pair(x, y);
}

// assumes all sky points are at infinity
inline double projectToSkyCylinder(cv::Point imgPoint) {
	return ((imgPoint.x - (cameraSize.width / 2)) * cameraHFOV) / (2*cameraSize.width); // (cameraHFOV/ 2) * ((imgPoint.x - (cameraSize.width / 2)) / cameraSize.width);
}

void visOdo_state::processSkyVectors() {
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY < skyBottom) {
			// project vectors to sky cylinder:
			i.skyFlag = 1;
			i[0].skyTheta = projectToSkyCylinder(i[0]);
			
			// update apparent rotation:
			i.rotTheta = (i.rotTheta + (i[0].skyTheta - i[1].skyTheta)) / 2;
		}
	}
}

void visOdo_state::processSkyVectors(double compassRot) {
	std::list<visOdo_feature> cTrackpoints;		// consistent
	std::list<visOdo_feature> icTrackpoints;	// inconsistent
	std::vector<visOdo_feature> newTrackpoints;
	
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY < skyBottom) {
			// project vectors to sky cylinder:
			i.skyFlag = 1;
			i[0].skyTheta = projectToSkyCylinder(i[0]);
			
			// update apparent rotation:
			i.rotTheta = (i.rotTheta + (i[0].skyTheta - i[1].skyTheta)) / 2;
			
			// filter out "inconsistent" trackpoints (that don't match compass readings)
			if(abs(i.rotTheta - compassRot) > vecUnsmoothThres) {
				icTrackpoints.push_back(std::move(i));
			} else {
				cTrackpoints.push_back(std::move(i));	
			}
		}
	}
	
	for(visOdo_feature& i : cTrackpoints) {
		newTrackpoints.push_back(std::move(i));
	}
	
	if(cTrackpoints.size() > icTrackpoints.size()) {
		for(visOdo_feature& i : icTrackpoints) {
			i.score += vecQualityPenalty;
			if(i.score < vecQualityCutoff) {
				newTrackpoints.push_back(std::move(i));
			}
		}
	} else {
		for(visOdo_feature& i : unsmoothTrackpoints) {
			newTrackpoints.push_back(std::move(i));
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
			
			// unrotate vectors:
			i[0].groundX = (i[0].groundX * cos(-this->last_rot)) - (i[0].groundY * sin(-this->last_rot));
			i[0].groundY = (i[0].groundX * sin(-this->last_rot)) + (i[0].groundY * cos(-this->last_rot));
			
			// update apparent translation:
			i.trnsX = (i.trnsX + (i[0].groundX - i[1].groundX)) / 2;
			i.trnsY = (i.trnsY + (i[0].groundY - i[1].groundY)) / 2;
			
			// filter by accelerometer data:
			if((abs(i.trnsX - tX) > vecInconsistentTrans) ||
				(abs(i.trnsY - tY) > vecInconsistentTrans)) {
				icTrackpoints.push_back(std::move(i));
			} else {
				cTrackpoints.push_back(std::move(i));	
			}
		}
	}
	
	for(visOdo_feature& i : cTrackpoints) {
		newTrackpoints.push_back(std::move(i));
	}
	
	if(cTrackpoints.size() > icTrackpoints.size()) {
		for(visOdo_feature& i : icTrackpoints) {
			i.score += vecQualityPenalty;
			if(i.score < vecQualityCutoff) {
				newTrackpoints.push_back(std::move(i));
			}
		}
	} else {
		for(visOdo_feature& i : unsmoothTrackpoints) {
			newTrackpoints.push_back(std::move(i));
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
			
			// unrotate vectors:
			i[0].groundX = (i[0].groundX * cos(-this->last_rot)) - (i[0].groundY * sin(-this->last_rot));
			i[0].groundY = (i[0].groundX * sin(-this->last_rot)) + (i[0].groundY * cos(-this->last_rot));
			
			// update apparent translation:
			i.trnsX = (i.trnsX + (i[0].groundX - i[1].groundX)) / 2;
			i.trnsY = (i.trnsY + (i[0].groundY - i[1].groundY)) / 2;
			
		}
	}
}

std::vector<visOdo_feature> visOdo_state::getAllSkyFeatures() {
	std::vector<visOdo_feature> rVec;
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY > skyBottom) {
			rVec.push_back(i);
		}
	}
	
	return rVec;
}

std::vector<visOdo_feature> visOdo_state::getAllGroundFeatures() {
	std::vector<visOdo_feature> rVec;
	for(visOdo_feature& i : this->trackpoints) {
		if(i[0].frmY < groundTop) {
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
		if(i.nVectors >= 7) {
			visOdo_vector& p7 = i[7];
			visOdo_vector& p3 = i[3];
			dir73 = atan2( p7.frmY - p3.frmY, p7.frmX - p3.frmX );
		}
		
		if(i.nVectors >= 3) {
			visOdo_vector& p3 = i[3];
			visOdo_vector& p1 = i[1];
			dir31 = atan2( p3.frmY - p1.frmY, p3.frmX - p1.frmX );
		}
		
		if(i.nVectors >= 2) {
			visOdo_vector& p1 = i[1];
			visOdo_vector& pc = i[0];
			dir31 = atan2( p1.frmY - pc.frmY, p1.frmX - pc.frmX );
		}
		
		// check for unsmooth vectors
		bool smooth = true;
		if(i.nVectors >= 7) {
			if(
			(abs(dir73 - dir31) > vecUnsmoothThres) ||
			(abs(dir73 - dir1c) > vecUnsmoothThres)
			) {
				smooth = false;
			}
		}
		
		if(i.nVectors >= 3) {
			if(abs(dir31 - dir1c) > vecUnsmoothThres) {
				smooth = false;
			}
		}
		
		i.score = max(i.score - vecQualityDecay, 0);
		
		if(!smooth) {
			smoothTrackpoints.push_back(std::move(i));
		} else {
			unsmoothTrackpoints.push_back(std::move(i));
		}
	}
	
	for(visOdo_feature& i : smoothTrackpoints) {
		newTrackpoints.push_back(std::move(i));
	}
	
	if(smoothTrackpoints.size() > unsmoothTrackpoints.size()) {
		// majority of elements are smooth, update scores
		for(visOdo_feature& i : unsmoothTrackpoints) {
			i.score += vecQualityPenalty;
			if(i.score < vecQualityCutoff) {
				newTrackpoints.push_back(std::move(i));
			}
		}
	} else {
		// don't update scores:
		for(visOdo_feature& i : unsmoothTrackpoints) {
			newTrackpoints.push_back(std::move(i));
		}
	}
	
	this->trackpoints = std::move(newTrackpoints);
}

void visOdo_state::findOpticalFlow(cv::Mat nextFrame) {
	std::vector<cv::Point> newPoints;
	std::vector<unsigned char> status;
	std::vector<unsigned char> err;
	
	// find new points
	cv::calcOpticalFlowPyrLK(this->lastFrame,
		nextFrame,
		this->lastPoints,
		newPoints,
		err);
	
	// match old points to new points
	unsigned int idx = 0;
	std::vector<visOdo_feature> newTrackpoints;
	for(cv::Point& i : this->lastPoints) {
		for(visOdo_feature& i2 : this->trackpoints) {
			if(i2[0] == i) {
				if(status[idx] == 1) {
					i2.addElement(newPoints[idx]);
					newTrackpoints.push_back(std::move(i2));
				}
			}
		}
		idx++;
	}
	
	this->trackpoints = std::move(newTrackpoints);
	this->lastPoints = std::move(newPoints);
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
	std::array<unsigned int, rotHistogramBinNum> bins;
	
	std::sort(skyVectors.begin(), skyVectors.end(), compareFeatureRotation);
	
	double histogramRange = skyVectors.front().rotTheta - skyVectors.back().rotTheta;
	double histogramBinSz = histogramRange / rotHistogramBinNum;
	double startRotTheta = skyVectors.front().rotTheta;
	
	for(visOdo_feature& i : skyVectors) {
		unsigned int binNum = ((startRotTheta - i.rotTheta) / histogramBinSz);
		
		bins[binNum]++;
	}
	
	double rotWeightedSum = 0;
	
	unsigned int idx = 0;
	double currentRotTheta = startRotTheta;
	for(unsigned int i : bins) {
		double weight = (i / skyVectors.size());
		
		rotWeightedSum = (i * currentRotTheta) / skyVectors.size();
		
		idx++;
		currentRotTheta += histogramBinSz;
	}
	
	this->last_rot = (rotWeightedSum + PI) - (floor((rotWeightedSum+PI) / PI) * PI);
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
	
	this->hdg += (this->last_rot * diff);
	this->posX += ((this->last_transX * cos(this->hdg)) * diff);
	this->posY += (this->last_transY * sin(this->hdg) * diff);
	
	
	this->lastTS = now;
}

void visOdo_state::startCycle(cv::Mat frame) {
	std::vector<cv::Point> newFeatures;
	cv::goodFeaturesToTrack(frame, newFeatures, nFeaturesTracked, minFeatureQuality, minDistFeatures);
	
	this->ttl = nFramesBetweenCycles;
	
	std::vector<visOdo_feature> newTrackpoints;
	for(cv::Point i : newFeatures) {
		newTrackpoints.emplace(newTrackpoints.end(), i);
	}
	
	this->trackpoints = std::move(newTrackpoints);
	this->lastPoints = std::move(newFeatures);
	this->lastFrame = frame;
	this->lastTS = std::chrono::steady_clock::now();
}

void visOdo_state::doCycle(cv::Mat frame, double compassRot, double tX, double tY) {
		if((this->ttl == 0) || (this->trackpoints.size < minNumFeatures)) {
			this->startCycle(frame);
		} else {
			this->findOpticalFlow(frame);
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
		if(this->ttl == 0 || (this->trackpoints.size < minNumFeatures)) {
			this->startCycle(frame);
		} else {
			this->findOpticalFlow(frame);
			this->filterUnsmoothVectors();
			this->processSkyVectors();
			this->findConsensusRotation();
			this->processGroundVectors();
			this->findConsensusTranslation();
			this->accumulateMovement();
		}
		
		this->ttl--;
}

void onTrackbarUpdate(unsigned int pos, void* ptr) {
	if(groundTop < skyBottom) {
		groundTop = skyBottom+1;
	}
}


int testVisOdo() {
	cv::namedWindow(processWindowName);
	cv::namedWindow(posWindowName);

	visOdo_state odoSt;

	cv::Mat currentVectorPos;
	cv::Mat posOutputWindow(cv::Size(800, 800), CV_8UC3);
	
	cv::VideoCapture cam(0);
	
	cameraSize = cv::Size(cam.get(CV_CAP_PROP_FRAME_WIDTH), cam.get(CV_CAP_PROP_FRAME_HEIGHT));
	
	cv::createTrackbar(groundTrackbarName, processWindowName, &groundTop, cameraSize.height, onTrackbarUpdate);
	cv::createTrackbar(skyTrackbarName, processWindowName, &skyBottom, cameraSize.height, onTrackbarUpdate);
	
	while(true) {
		
		if(!cam.open())
			return -1;

		cv::Mat img;
		cam >> img;

		if(odoSt.ttl == 0) {
			currentVectorPos = cv::Mat::zeros(img.size(), CV_8UC3);
		}
		
		double t = (double)cv::getTickCount();
		
		odoSt.doCycle(img);

		double fps = 1 / (((double)cv::getTickCount() - t) / cv::getTickFrequency());
		
		cv::Mat copy = img.clone();
		
		for(visOdo_feature& i : odoSt.trackpoints) {
				currentVectorPos.at((cv::Point)i.history[0]) = vectorColor;
				cv::circle(copy, (cv::Point)i.history[0], vectorDrawSz);
		}
		
		copy.setTo(currentVectorPos, currentVectorPos);
		
		if(odoSt.ttl == 0) {
			unsigned int posXft = (unsigned int)(odoSt.posX * 3.28084);
			unsigned int posYft = (unsigned int)(odoSt.posY * 3.28084);
			
			posOutputWindow.at(cv::Point(posXft+400, posYft+400)) = posColor;
		}
		
		cv::line(copy, cv::Point(0, groundTop), cv::Point(cameraSize.width-1, groundTop), groundColor);
		cv::line(copy, cv::Point(0, skyBottom), cv::Point(cameraSize.width-1, skyBottom), skyColor);
		
		cv::putText(copy, "current fps: " + std::to_string(fps), cv::Point(50, 25),FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "current heading: " + std::to_string(odoSt.hdg), cv::Point(50, 50),FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "number features: " + std::to_string(odoSt.trackpoints.size()), cv::Point(50, 75),FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		
		cv::putText(copy, "velX:  " + std::to_string(odoSt.last_transX), cv::Point(50, 125),FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "velY:  " + std::to_string(odoSt.last_transY), cv::Point(50, 150),FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		cv::putText(copy, "velRot: " + std::to_string(odoSt.last_rot), cv::Point(50, 175),FONT_HERSHEY_SIMPLEX, 1.0, textColor);
		
		cv::imshow(processWindowName, copy);
		cv::imshow(posWindowName, posOutputWindow);
		
		if(cv::waitKey() > 0) {
			break;
		}
	}
}
