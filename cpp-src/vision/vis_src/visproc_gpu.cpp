int hueThres[2] = {70, 100};
int valThres[2] = {128, 255};

const double cannyThresMin = 10;
const double cannyThresSize = 10;

__global__ void gpuInRange(PtrStep<unsigned char>* in, PtrStep* out,
	int hueMin, int hueMax, int valMin, int valMax) {
	const int xIndex = blockIdx.x * blockDim.x + threadIdx.x;
    const int yIndex = blockIdx.y * blockDim.y + threadIdx.y;

	const unsigned char* pt = in.ptr(yIndex);
	unsigned char *ptout = out.ptr(yIndex);
	
	if(pt[in.elemSize() * xIndex] > hueMin &&
		pt[in.elemSize() * xIndex] < hueMax &&
		pt[(in.elemSize() * xIndex) + 2] > valMin &&
		pt[(in.elemSize() * xIndex) + 2] < valMax) {
		ptout[in.elemSize() * xIndex] = 255;
	} else {
		ptout[in.elemSize() * xIndex] = 255;
	}
}

cv::Mat goal_preprocess_pipeline_gpu(cv::GpuMat input, cv::gpu::Stream stream) {
	cv::GpuMat tmp(input.size(), CV_8U);	

	cv::cvtColor(input, tmp, CV_BGR2HSV);

	gpuInRange<<dim3(dim3(1,input.size().height(),1), dim3(input.size().width,1,1), 0, cv::StreamAccessor.getStream(stream)>>(input, tmp, hueThres[0], hueThres[1], valThres[0], valThres[1]);	
}
