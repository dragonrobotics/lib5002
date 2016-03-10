#pragma once
#include "opencv2/core.hpp"
#include "netaddr.h"
#include "sockwrap.h"

/*!	\file wpilib_cameraserver.h
 *	\brief Provides an interface for working with the vanilla WPILib CameraServer streams.
 */

const unsigned int cs_port = 1180; //!< TCP port used by the WPILib Camera Server
const unsigned char cs_header[4] = { 0x01, 0x00, 0x00, 0x00 }; //!< 4-byte header used by the camera server for each sent packet.

/*!	\enum cs_imgSize
 *	\brief Image sizes sent to the CameraServer initial connection request.
 */
enum class cs_imgSize : unsigned int {
	SZ_640x480 = 0, //!< 640 x 480 pixels
	SZ_320x240 = 1, //!< 320 x 240 pixels
	SZ_160x120 = 2	//!< 160 x 120 pixels
};

extern connSocket connectToCamServer(netaddr serverAddress, int fps, cs_imgSize sz);
extern cv::Mat getImageFromServer(netaddr serverAddress);
extern cv::Mat getImageFromServer(connSocket& cs_socket);
extern void setStreamSettings(connSocket& visConn, int fps, cs_imgSize sz);
