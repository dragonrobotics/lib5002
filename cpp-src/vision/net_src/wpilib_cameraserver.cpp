#include "wpilib_cameraserver.h"
#include "network_bytestream.h"
#include "opencv2/highgui.hpp"

/*
 * WPILib CameraServer network format:
 * Port: TCP 1180
 * 
 * Server waits for client to transmit:
 * - desired FPS
 * - desired compression (only accepted value: -1)
 * - desired size (out of cs_imgSize enum)
 * Server then continually transmits:
 * - Protocol header (4 bytes): 0x01 0x00 0x00 0x00
 * - Data length (4 bytes), int
 * - JPEG image data
 */

connSocket connectToCamServer(netaddr serverAddress,
	int fps, cs_imgSize sz) {

	serverAddress.setPort(cs_port);
	connSocket cs_connection(serverAddress);
	
	setStreamSettings(cs_connection, fps, sz);

	return cs_connection;
}

void setStreamSettings(connSocket& visConn, int fps, cs_imgSize sz) {
	nbstream opening;
	opening.put32(fps);
	opening.put32(static_cast<uint32_t>(-1));
	opening.put32(static_cast<unsigned int>(sz));
	netmsg msg(opening.tobuf(), opening.getbufsz());

	visConn.send(msg);
}

cv::Mat getImageFromServer(connSocket& cs_socket) {
	while(true) {
		nbstream headerStream;
		while(true) {
			std::cout << "Waiting on header data..." << std::endl;
			netmsg headerData = cs_socket.recv_n(8); // 600 * 480 * 3 bytes of JPEG data at max + 8 bytes of header
			
			headerStream = nbstream(headerData.getbuf(), headerData.getbufsz());

			unsigned char header[4] = { headerStream.get8(), headerStream.get8(), headerStream.get8(), headerStream.get8() };

			bool valid_header = true;
			for(int i=0;i<4;i++) {
				if(header[i] != cs_header[i]) {
					valid_header = false;
					break;		
				}
			}

			if(valid_header)
				break;
		}

		int sz = headerStream.get32();
		
		std::cout << "Received " << sz << " bytes." << std::endl;

		netmsg payload = cs_socket.recv_n(sz);
		//nbstream dataStream(payload.getbuf(), payload.getbufsz());

		unsigned char* bufptr = payload.getbuf().get();

		std::vector<unsigned char> jpegdata(bufptr, bufptr+payload.getbufsz()); //jpegdata(dataStream.cur, dataStream.buf.end());
		return cv::imdecode(jpegdata, CV_LOAD_IMAGE_COLOR);
	}
}
