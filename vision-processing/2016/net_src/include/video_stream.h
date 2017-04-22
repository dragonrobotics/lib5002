#pragma once
#include "network_bytestream.h"
#include "msgtype.h"
#include "opencv2/core.hpp"

/*!	\file video_stream.h
 *	\brief Defines packet types for custom raw video streams.
 */

/*!	\enum color_fmt
 *	\brief Defines color formats supported by the stream receiver.
 */
enum class color_fmt : unsigned char {
	BGR = 0,
	RGB = 1,
	HSV = 2,
	HSL = 3,
	BIN = 4,
	UNKNOWN = 5,
};

/*!	\class video_stream_msg
 *	\brief Defines a raw video stream packet.
 */

struct video_stream_msg : public message_payload { // TODO: Maybe add compression of some kind?
	/*

	Actual Binary Stream Layout:

	unsigned short xSize		: 
	unsigned short ySize		: sizes as returned by Mat::size().(x/y)
	unsigned int openCVType		: type as returned by Mat::type()
	color_fmt format		: color format (RGB / HSV)
	void *videoData			: Raw video data, continuously stored (row 0, then row 1 immediately after)

	*/

	cv::Mat img;		//!< cv::Mat containing the transmitted video data.
	color_fmt format;	//!< Color format of the contained video data.

	/*!	\fn video_stream_msg(cv::Mat transMat)
	 *	\brief Constructs a video stream packet from a given OpenCV matrix.
	 */
	video_stream_msg(cv::Mat transMat) : img(transMat) {};

	message_type typeof_data() { return message_type::VIDEO_STREAM; };
	void tobuffer(nbstream& stream);
	void frombuffer(nbstream& stream);
};
