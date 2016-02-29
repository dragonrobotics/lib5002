/*
 * Visual Odometry
 * Written by: Sebastian Mobo
 * 2/29/2016
 * An implementation of the monocular visual odometry system by Campbell, et al.
 * See: https://www.cs.cmu.edu/~personalrover/PER/ResearchersPapers/CampbellSukthankarNourbakhshPahwa_VisualOdometryCR.pdf
 *
 */

const double cameraTiltAngle = 0.0; // 90 degrees from ground (principal ray parallel to ground plane)
const double cameraVFOV = ??; // Camera vertical FOV in radians.
const double cameraHFOV = ??; // Camera horizontal FOV in radians.
const double cameraHeight = 0; // Height of camera off ground in meters.

cv::Size cameraSize;

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

