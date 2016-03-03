typedef static_tp std::chrono::time_point<std::chrono::steady_clock>;

/*
* HANDY CONSTANTS
*/

cv::Size cameraSize;

// laptop webcam:
// width on wall: ~6 ft = ~72 in.
// Distance from wall: ~63 1/2 in. (5.29 ft.)
// approximate FOV:
//	59.1 degrees horizontal
//	35.4 degrees vertical
// 	66.1 degrees diagonal
// focal length: 3.53 mm

// far measurements:
// width on wall: ~128 in. (10.6 ft.)
// distance from wall: ~111 in. (9.25 ft.)
// approximate FOV:
//	59.9 degrees horizontal
//	35.9 degrees vertical
// 	67.0 degrees diagonal
// Focal length: 3.47 mm

// Average focal length: 3.14359215092 mm

// final FOV: 61.04 degrees (horizontal)
// effective focal length: 30.54 in.

// frame dimensions: 1280 x 720 pixels
// v = 2atan(tan(h/2) x h/w)
// v = 2atan(tan(61.04 / 2) * (1280 / 720)
// v = 2atan(tan(61.04 / 2) * (16/9))

const double PI = 3.14159265359;

/*
* CAMERA PARAMETERS
*/ 
const double cameraTiltAngle = 0.0; // 90 degrees from ground (principal ray parallel to ground plane)
const double cameraVFOV = 35.4 * (PI / 180.0); // Camera vertical FOV in radians.
const double cameraHFOV = 59.1 * (PI / 180.0); // Camera horizontal FOV in radians.
const double cameraHeight = 0; // Height of camera off ground in meters.

/*
* ALGORITHM PARAMETERS
*/
const unsigned int nFeaturesTracked = 100;
const double minFeatureQuality = 0.005;
const unsigned int minDistFeatures = 20; // pixels

const unsigned int nFramesBetweenCycles = 10;

const double vecUnsmoothThres = (PI / 6.0); // reject flow vectors that have flow directions that differ by this many radians (30 deg * PI / 180 = PI / 6 rad)

const double vecInconsistentTrans = 0.1; // reject flow vectors that differ from accelerometer readings by this many meters

const unsigned int vecQualityCutoff = 10;	// discard features that have unsmoothness scores above this threshold
const unsigned int vecQualityPenalty = 5;	// unsmooth flow vectors have this added to their scores
const unsigned int vecQualityDecay  = 1;	// every cycle removes this from each feature's score

const unsigned int vecHistoryLen = 7;

unsigned int groundTop = 240;			// beginning of horizon zone
unsigned int skyBottom = 600;			// end of horizon zone

const unsigned int consensusRandGroupSz = 40;	// number of elements to get consensus for

const unsigned int rotHistogramBinNum	= 1000;	// number of bins in rotational histogram


struct visOdo_vector {
	unsigned short frmX;
	unsigned short frmY;
	
	double groundX;
	double groundY;
	double skyTheta;
	
	operator cv::Point() { return cv::Point(frmX, frmY); };
	visOdo_vector(cv::Point frm) : frmX(frm.x), frmY(frm.y) {};
} __attribute__((packed)); // size: 28 bytes

struct visOdo_feature {
	visOdo_vector history[vecHistoryLen+1];
	unsigned char historyHead = 0;
	unsigned char nVectors = 0;
	
	double rotTheta;
	double trnsX;
	double trnsY;
	
	unsigned char delFlag;	// 1 == delete, 0 == don't delete
	unsigned char skyFlag;	// 1 == sky vector, 0 == ground vector
	unsigned short score;
	
	// get vector from n elements ago, n must be < nVectors
	visOdo_vector& operator[](unsigned int n) {
		return this->history[((vecHistoryLen+1) + (historyHead - n)) % (vecHistoryLen+1)];
	}
	
	void addElement(visOdo_vector& elem) {
		historyHead = (historyHead+1) % (vecHistoryLen+1);
		history[historyHead] = elem;
		nVectors = min(nVectors+1, vecHistoryLen+1);
	}
	
}  __attribute__((packed)); // size: (28 * vecHistoryLen+1) + 6 + (8*3) or 202 bytes if vecHistoryLen == 8

struct visOdo_state {
	cv::Mat lastFrame;
	std::vector<visOdo_feature> trackpoints;
	std::vector<cv::Point> lastPoints;
	unsigned int ttl = 0;
	
	double last_transX;
	double last_transY;
	double last_rot;
	
	double posX;
	double posY;
	double hdg;
	
	static_tp lastTS;
	
	void startCycle(cv::Mat frame); // find good features to track, set ttl, fill trackpoints and lastPoints and lastFrame
	
	void findOpticalFlow(cv::Mat frame);
	void filterUnsmoothVectors();

	void processSkyVectors(double compassRot);
	void processGroundVectors(double tX, double tY);

	void processSkyVectors();
	void processGroundVectors();

	void findConsensusRotation();
	void findConsensusTranslation();
	void accumulateMovement();
	
	void doCycle(cv::Mat frame, double compassRot, double tX, double tY);
	void doCycle(cv::Mat frame);	

	std::vector<visOdo_feature> getAllSkyFeatures();
	std::vector<visOdo_feature> getAllGroundFeatures();
};
