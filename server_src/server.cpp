// simple server stub to handle requests for information

#include "sockwrap.h"
#include "msgtype.h"
#include "visproc_interface.h"
#include <iostream>

const int serverPort = 5800;
const int connType = SOCK_STREAM; // TCP

std::unique_ptr<message> create_message_buffer(void* payload, size_t payloadsz) {
	unsigned char* buf = new unsigned char[sizeof message + payloadsz];
	message* out = static_cast<message*>(buf);
	
	out->message();
	memcpy(buf+sizeof(message)-1, payload, payloadsz);
	
	return std::unique_ptr<message>(out);
}

void do_server() {
	serverSocket sock(serverPort, connType);
	cv::VideoCapture camera(0);
	
	if(!camera.isOpened()) {
		std::cerr << "Could not open camera." << std::endl;
		return;
	}
	
	while(true) {
		netmsg msg = sock.recv(0);
		
		std::shared_ptr<message> msgdata =
			std::static_pointer_cast(msg.data);
			
		if(msgdata->header[0] == '5' && msgdata->header[1] == '0' &&
			msgdata->header[2] == '0' && msgdata->header[3] == '2') {
			if(msgdata->type == message_types.GET_GOAL_DISTANCE) {
				cv::Mat src;
				
				camera >> src;
				
				scoredContour out = goal_pipeline(goal_preprocess_pipeline(src));
				double dist = -1;
				if( out.second.size() > 0 ) {
					cv::Rect bounds = cv::boundingRect(out.second);
					dist = getDistance(bounds.size(), src.size());
				}
				
				goal_distance_msg ret(dist, out.first);
				
				std::unique_ptr<void> ret_data = ret.struct_to_data();
				
				short sz = ret.sizeof_data();
					
				std::unique_ptr<message> packet =
					create_message_buffer(ret_data.get(), sz);
				
				netmsg retmsg(conntype);
				retmsg.setbuf(packet, sz);
				
				sock.send(retmsg);
			}
		}
			
	}
}