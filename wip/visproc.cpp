const double hueThres[2] = {0,0} /*Hue threshold*/
const double valThres[2] = {0,0}

Mat pipeline(Mat& input) {
    Mat tmp(input.size(), input.type());
    
    cvtColor(input, tmp, CV_BGR2HSV); 
    
    Mat mask(input.size(), input.type());
    
    /* Filter on color/brightness */
    inRange(tmp, Scalar(hueThres[0],0,valThres[0]), Scalar(hueThres[1],255,valThres[1]), mask);
    
    
}
