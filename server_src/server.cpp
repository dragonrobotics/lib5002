// simple server stub to handle requests for information

#include "sockwrap.h"
#include "msgtype.h"
#include "visproc_interface.h"
#include <iostream>

const int serverPort = 5800;

void disc_server() {
	serverSocket sock(serverPort, SOCK_DGRAM);
	std::cout << "Listening on " << sock.getbindaddr() << std::endl;

	while(true) {
		netmsg msg = sock.recv(0);
		
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get())) {
			std::shared_ptr<message> msgdata = std::static_pointer_cast(msg.getbuf());
			if(msgdata->type == message_types::DISCOVER) {
				std::cout << "Received DISCOVER message from " << msg.addr << std::endl;
				netmsg out = wrap_packet(discover_msg(discover_msg::origin_t::JETSON), SOCK_DGRAM);
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
		
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get())) {
			std::shared_ptr<message> msgdata = std::static_pointer_cast(msg.getbuf());

			if(msgdata->type == message_types.GET_GOAL_DISTANCE) {
				cv::Mat src;
				
				camera >> src;
				
				scoredContour out = goal_pipeline(goal_preprocess_pipeline(src));
				double dist = -1;
				if( out.second.size() > 0 ) {
					cv::Rect bounds = cv::boundingRect(out.second);
					dist = getDistance(bounds.size(), src.size());
				}
				
				sock.send(wrap_packet(goal_distance_msg(dist, out.first), SOCK_STREAM));
			}
		}			
	}
}

int main() {
	disc_server();
}
