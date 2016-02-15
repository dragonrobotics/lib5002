#pragma once
#include "opencv2/core.hpp"
#include "netaddr.h"
#include "sockwrap.h"

const unsigned int cs_port = 1180; // TCP
const unsigned char cs_header[4] = { 0x01, 0x00, 0x00, 0x00 };

enum class cs_imgSize : unsigned int {
	SZ_640x480 = 0,
	SZ_320x240 = 1,
	SZ_160x120 = 2
};

extern cv::Mat getImageFromServer(netaddr serverAddress);
extern cv::Mat getImageFromServer(connSocket& cs_socket);

