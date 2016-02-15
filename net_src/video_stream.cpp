#include "video_stream.h"
#include "opencv2/highgui.hpp"


/* ----------------------------------------------------------------- */
/*			class video_stream_msg			     */
/* ----------------------------------------------------------------- */

void video_stream_msg::tobuffer(nbstream& stream) {
	int xSize = this->img.size().width;
	int ySize = this->img.size().height;	
	int nChannels = this->img.channels();

	stream.setbufsz(7 + (xSize * ySize * nChannels) );

	stream.put16((short)xSize);
	stream.put16((short)ySize);
	stream.put32(nChannels);
	stream.put8(static_cast<unsigned char>(this->format));
	
	void* videoData = stream.getrawptr()+8;

	int rows = this->img.rows;
	int cols = this->img.cols * nChannels;

	if(this->img.isContinuous()) {
		cols *= rows;
		rows = 1;
	}
	
	for(int i=0;i<rows;i++) {
		unsigned char* matPtr = this->img.ptr<unsigned char>(i);
		unsigned char* arrPtr = static_cast<unsigned char*>(videoData + (i * cols));
		memcpy(arrPtr, matPtr, i * cols);
	}
}

void video_stream_msg::frombuffer(nbstream& stream) {
	int xSize = stream.get16();
	int ySize = stream.get16();
	int openCVType = stream.get32();
	unsigned char fmtbyte = stream.get8();
	this->format = (fmtbyte < 5) ? static_cast<color_fmt>(fmtbyte) : color_fmt::UNKNOWN;

	if(openCVType == 0) {
		std::vector<unsigned char> rawData(stream.cur, stream.buf.end());
		this->img = cv::imdecode(rawData, CV_LOAD_IMAGE_COLOR);
	} else {
		this->img = cv::Mat(xSize, ySize, openCVType);
		int rows = this->img.rows;
		int cols = this->img.cols * this->img.channels();

		void* videoData = stream.getrawptr()+8;

		for(int i=0;i<rows;i++) {
			unsigned char* matPtr = this->img.ptr<unsigned char>(i);
			unsigned char* arrPtr = static_cast<unsigned char*>(videoData + (i * cols));
			memcpy(matPtr, arrPtr, i * cols);
		}
	}
}

