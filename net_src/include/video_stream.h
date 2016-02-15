#pragma once
#include "network_bytestream.h"
#include "msgtype.h"
#include "opencv2/core.hpp"

enum class color_fmt : unsigned char {
	BGR = 0,
	RGB = 1,
	HSV = 2,
	HSL = 3,
	BIN = 4,
	UNKNOWN = 5,
};

struct video_stream_msg : public message_payload { // TODO: Maybe add compression of some kind?
	/*

	Actual Binary Stream Layout:

	unsigned short xSize		: 
	unsigned short ySize		: sizes as returned by Mat::size().(x/y)
	unsigned int openCVType		: type as returned by Mat::type()
	color_fmt format		: color format (RGB / HSV)
	void *videoData			: Raw video data, continuously stored (row 0, then row 1 immediately after)

	*/

	cv::Mat img;
	color_fmt format;

	video_stream_msg(cv::Mat transMat) : img(transMat) {};
	message_type typeof_data() { return message_type::VIDEO_STREAM; };
	void tobuffer(nbstream& stream);
	void frombuffer(nbstream& stream);
};
