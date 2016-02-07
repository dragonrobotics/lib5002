// simple server stub to handle requests for information

#include "sockwrap.h"
#include "msgtype.h"
#include "visproc_interface.h"
#include <iostream>
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"

const int serverPort = 5800;

void disc_server() {
	serverSocket sock(serverPort, SOCK_DGRAM);
	std::cout << "Listening on " << (std::string)sock.getbindaddr() << std::endl;

	while(true) {
		netmsg msg = sock.recv(0);
		
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			std::shared_ptr<message> msgdata(reinterpret_cast<message*>(msg.getbuf().get()));
			if(msgdata->type == message_type::DISCOVER) {
				std::cout << "Received DISCOVER message from " << (std::string)msg.addr << std::endl;
				discover_msg retm(discover_msg::origin_t::JETSON);
				netmsg out = message::wrap_packet(&retm, SOCK_DGRAM);
				out.addr = msg.addr;

				sock.send(out);
			}
		}
	}
}

void conn_server() {
	serverSocket listenSock(serverPort, SOCK_STREAM);
	
	cv::VideoCapture camera(0);
	
	if(!camera.isOpened()) {
		std::cerr << "Could not open camera." << std::endl;
		return;
	}
	
	while(true) {
		connSocket dataSock = listenSock.waitForConnection();
		netaddr addr = dataSock.getaddr();
		
		while(true) {
			netmsg msg = dataSock.recv(0);

			if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
				std::shared_ptr<message> msgdata(reinterpret_cast<message*>(msg.getbuf().get()));

				if(msgdata->type == message_type::GET_GOAL_DISTANCE) {
					cv::Mat src;
				
					camera >> src;
				
					scoredContour out = goal_pipeline(goal_preprocess_pipeline(src));
					double dist = -1;
					if( out.second.size() > 0 ) {
						cv::Rect bounds = cv::boundingRect(out.second);
						dist = getDistance(bounds.size(), src.size());
					}
				
					goal_distance_msg retm(dist, out.first);
					dataSock.send(message::wrap_packet(&retm, SOCK_STREAM));
				}
			}	
		}		
	}
}

int main() {
	disc_server();
	return 0;
}
