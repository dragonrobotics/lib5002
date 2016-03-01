/*
 * Visual Odometry
 * Written by: Sebastian Mobo
 * 2/29/2016
 * An implementation of the monocular visual odometry system by Campbell, et al.
 * See: https://www.cs.cmu.edu/~personalrover/PER/ResearchersPapers/CampbellSukthankarNourbakhshPahwa_VisualOdometryCR.pdf
 *
 */

/*
 * CAMERA PARAMETERS
 */ 
const double cameraTiltAngle = 0.0; // 90 degrees from ground (principal ray parallel to ground plane)
const double cameraVFOV = ??; // Camera vertical FOV in radians.
const double cameraHFOV = ??; // Camera horizontal FOV in radians.
const double cameraHeight = 0; // Height of camera off ground in meters.

/*
 * ALGORITHM PARAMETERS
 */
const double vecUnsmoothThres = (PI / 6.0); // reject flow vectors that have flow directions that differ by this many radians (30 deg * PI / 180 = PI / 6 rad)

const unsigned int vecQualityCutoff = 10;	// discard features that have unsmoothness scores above this threshold
const unsigned int vecQualityPenalty = 5;	// unsmooth flow vectors have this added to their scores
const unsigned int vecQualityDecay  = 1;	// every cycle removes this from each feature's score

unsigned int groundTop = 240;			// beginning of horizon zone
unsigned int skyBottom = 600;			// end of horizon zone

cv::Size cameraSize;

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

cv::Point projectToGroundPlane(cv::Point imgPoint) {
	double angleToGround = atan((2*imgPoint.y - cameraSize.height) * tan(cameraVFOV/2));
	//double angleX = atan((2*imgPoint.x - cameraSize.width) * tan(cameraHFOV/2));
	
	double y = height / tan(angleToGround+cameraTiltAngle);
	
	double fovLenAtY = tan(cameraHFOV) / (2 * y);
	double x = fovLenAtY * ((imgPoint.x - (cameraSize.width / 2)) / cameraSize.width);
	
	return cv::Point(x, y);
}

// assumes all sky points are at infinity
double projectToSkyCylinder(cv::Point imgPoint) {
	return ((imgPoint.x - (cameraSize.width / 2)) * cameraHFOV) / (2*cameraSize.width); // (cameraHFOV/ 2) * ((imgPoint.x - (cameraSize.width / 2)) / cameraSize.width);
}

/*
 * Do filtering for unsmooth vectors and update quality scores.
 */
void filterTUnsmoothVectors(visOdo_state& inputState) {
	double accDir = atan2(vY, vX);
	
	std::list<visOdo_feature> smoothTrackpoints;
	std::list<visOdo_feature> unsmoothTrackpoints;
	std::list<visOdo_feature> newTrackpoints;
	
	for(visOdo_feature& i : inputState.trackpoints) {
		// end - 1 = current frame, end - 2 = back 1 frame, etc...
		double dir73 = 0;
		double dir31 = 0;
		double dir1c = 0;
		
		// calculate vector direction values
		if(i.point_history.size() >= 8) {
			cv::Point p7 = *(i.point_history.end()-8);
			cv::Point p3 = *(i.point_history.end()-4);
			dir73 = atan2( p7.y - p3.y, p7.x - p3.x );
		}
		
		if(i.point_history.size() >= 4) {
			cv::Point p3 = *(i.point_history.end()-4);
			cv::Point p1 = *(i.point_history.end()-2);
			dir31 = atan2( p3.y - p1.y, p3.x - p1.x );
		}
		
		if(i.point_history.size() >= 2) {
			cv::Point p1 = *(i.point_history.end()-2);
			cv::Point pCur = *(i.point_history.end()-1);
			dir1c = atan2( p1.y - pCur.y, p1.x - pCur.x );
		}
		
		// check for unsmooth vectors
		bool smooth = true;
		if(i.point_history.size() >= 8) {
			if(
			(abs(dir73 - dir31) > vecUnsmoothThres) ||
			(abs(dir73 - dir1c) > vecUnsmoothThres)
			) {
				smooth = false;
			}
		}
		
		if(i.point_history.size() >= 4) {
			if(abs(dir31 - dir1c) > vecUnsmoothThres) {
				smooth = false;
			}
		}
		
		i.score -= vecQualityDecay;
		
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
	}
	
	inputState.trackpoints = std::move(newTrackpoints);
}

void findOpticalFlow(visOdo_state& ioState, cv::Mat nextFrame) {
	std::vector<cv::Point> newPoints;
	std::vector<unsigned char> status;
	std::vector<unsigned char> err;
	
	// find new points
	cv::calcOpticalFlowPyrLK(ioState.lastFrame,
		nextFrame,
		ioState.lastPoints,
		newPoints,
		err);
	
	// match old points to new points
	unsigned int idx = 0;
	std::list<visOdo_feature> newTrackpoints;
	for(cv::Point& i : ioState.lastPoints) {
		for(visOdo_feature& i2 : ioState.trackpoints) {
			if(i2.point_history.back() == i) {
				if(status[idx] == 1) {
					i2.point_history.push_back(newPoints[idx]);
					newTrackpoints.push_back(std::move(i2));
				}
			}
		}
		idx++;
	}
	
	ioState.trackpoints = std::move(newTrackpoints);
	ioState.lastPoints = std::move(newPoints);
	ioState.lastFrame = nextFrame;
	ioState.ttl--;
}